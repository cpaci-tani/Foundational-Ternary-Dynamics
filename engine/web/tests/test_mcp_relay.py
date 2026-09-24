"""Real loopback routes and deterministic mailbox lifecycle/security checks."""
from concurrent.futures import Future
from email.message import Message
import json
import threading
from types import SimpleNamespace

import pytest
from test_strict_hydro_routes import local_server, serve

relay_module = serve.mcp_relay
BROWSER = {"Content-Type": "application/json", "X-FTD-MCP": "1"}


class Clock:
    value = 1000.0

    def __call__(self):
        return self.value

    def wall(self):
        return 1700000000 + self.value

    def advance(self, seconds):
        self.value += seconds


@pytest.fixture
def mailbox(monkeypatch):
    clock = Clock()
    relay = relay_module.Relay(clock=clock, wall_clock=clock.wall, poll_seconds=0, call_seconds=30)
    monkeypatch.setattr(relay_module, "RELAY", relay)
    yield relay, clock
    with relay.condition:
        for session in list(relay.sessions.values()):
            relay._close(session, "test_shutdown")


def start_call(relay, session, identifier="one", method="observe", args=None):
    result = Future()

    def call():
        try:
            result.set_result(relay.call(session["sessionId"], session["clientToken"], identifier, method, args or {}))
        except BaseException as error:
            result.set_exception(error)

    thread = threading.Thread(target=call, daemon=True)
    thread.start()
    with relay.condition:
        assert relay.condition.wait_for(lambda: identifier in relay.sessions[session["sessionId"]].calls or result.done(), 2)
    return result


def poll(relay, session):
    return relay.poll(session["sessionId"], session["browserToken"])


def reply(relay, session, identifier="one", value=None):
    return relay.reply(session["sessionId"], session["browserToken"], identifier, {"result": value})


def post(request, operation, body, headers=None):
    status, response_headers, raw = request("/api/mcp/" + operation, "POST", json.dumps(body), headers or BROWSER)
    return status, response_headers, json.loads(raw)


def test_browser_registration_rejects_foreign_host_origin_and_fetch_site(local_server, mailbox):
    request = local_server[2]
    for headers in ({"Host": "attacker.example"}, {"Host": "localhost:1"},
                    {"Host": "user@localhost:80"}, {"Origin": "https://attacker.example"},
                    {"Origin": "null"}, {"Sec-Fetch-Site": "same-site"}, {"Sec-Fetch-Site": "cross-site"}):
        assert post(request, "register", {"label": "browser"}, {**BROWSER, **headers})[0] == 403
    assert post(request, "register", {"label": "browser"}, {"Content-Type": "application/json"})[0] == 403
    status, headers, session = post(request, "register", {"label": "browser"})
    assert status == 200 and session["browserToken"] != session["clientToken"]
    assert len(session["browserToken"]) >= 43 and len(session["clientToken"]) >= 43
    assert session["serverPath"].replace("\\", "/").endswith("/engine/mcp/server.mjs")
    assert "access-control-allow-origin" not in headers and "no-store" in headers["cache-control"]
    assert request("/api/mcp/register")[0] == 405


def test_browser_and_client_credentials_are_separate_and_session_scoped(local_server, mailbox):
    request = local_server[2]
    first = post(request, "register", {"label": "one"})[2]
    second = post(request, "register", {"label": "two"})[2]
    for token in (first["clientToken"], second["browserToken"], "wrong"):
        assert post(request, "poll", {"sessionId": first["sessionId"], "browserToken": token})[0] == 401
    body = {"sessionId": first["sessionId"], "id": "one", "method": "observe", "args": {}}
    for token in (first["browserToken"], second["clientToken"], "wrong"):
        assert post(request, "call", body, {"Content-Type": "application/json", "Authorization": "Bearer " + token})[0] == 401
    assert post(request, "call", body, {"Content-Type": "application/json"})[0] == 401
    assert post(request, "poll", {"sessionId": first["sessionId"], "browserToken": first["browserToken"]})[2] == {"requests": [], "cancelled": []}


def test_idempotent_delivery_waits_for_one_reply_and_conflicting_payload_never_requeues(mailbox):
    relay, clock = mailbox
    session = relay.register("one")
    first = start_call(relay, session, args={"a": 1, "b": 2})
    duplicate = start_call(relay, session, args={"b": 2, "a": 1})
    with pytest.raises(relay_module.RelayError) as conflict:
        relay.call(session["sessionId"], session["clientToken"], "one", "observe", {"a": 3})
    assert conflict.value.status == 409
    message = poll(relay, session)
    assert message == {"requests": [{"id": "one", "method": "observe", "args": {"a": 1, "b": 2},
                                       "deadline": int((clock.wall() + 30) * 1000)}], "cancelled": []}
    assert poll(relay, session)["requests"] == []
    assert not first.done() and not duplicate.done()
    reply(relay, session, value={"ownerId": "original"})
    expected = (200, {"result": {"ownerId": "original"}})
    assert first.result(2) == duplicate.result(2) == expected
    assert relay.call(session["sessionId"], session["clientToken"], "one", "observe", {"a": 1, "b": 2}) == expected
    assert reply(relay, session, value={"ownerId": "original"}) == {"accepted": True}
    with pytest.raises(relay_module.RelayError):
        reply(relay, session, value={"ownerId": "changed"})
    assert poll(relay, session)["requests"] == []


@pytest.mark.parametrize("delivered", [False, True])
def test_cancellation_purges_undelivered_and_marks_inflight_unknown_without_retry(mailbox, delivered):
    relay, _ = mailbox
    session = relay.register("one")
    future = start_call(relay, session, method="execute_plan")
    if delivered:
        assert len(poll(relay, session)["requests"]) == 1
    relay.cancel(session["sessionId"], session["clientToken"], "one")
    status, body = future.result(2)
    assert status == 409 and body["error"]["outcome"] == ("unknown" if delivered else "not_dispatched")
    assert poll(relay, session) == {"requests": [], "cancelled": ["one"]}
    assert relay.call(session["sessionId"], session["clientToken"], "one", "execute_plan", {}) == (status, body)
    assert poll(relay, session)["requests"] == []
    with pytest.raises(relay_module.RelayError):
        reply(relay, session)


def test_poll_receives_cancellation_while_browser_command_is_still_inflight(mailbox):
    relay, _ = mailbox
    relay.poll_seconds = 20
    session = relay.register("one")
    call = start_call(relay, session, method="run_scenario")
    assert len(poll(relay, session)["requests"]) == 1
    received = Future()
    threading.Thread(target=lambda: received.set_result(poll(relay, session)), daemon=True).start()
    with relay.condition:
        assert relay.condition.wait_for(lambda: relay.sessions[session["sessionId"]].polling, 2)
    relay.cancel(session["sessionId"], session["clientToken"], "one")
    assert received.result(2) == {"requests": [], "cancelled": ["one"]}
    assert call.result(2)[1]["error"]["outcome"] == "unknown"


def test_cancellation_overtaking_call_prevents_late_dispatch_and_binds_id(mailbox, monkeypatch):
    relay, _ = mailbox
    session = relay.register("one")
    assert relay.cancel(session["sessionId"], session["clientToken"], "early")["outcome"]["error"]["outcome"] == "not_dispatched"
    status, body = relay.call(session["sessionId"], session["clientToken"], "early", "execute_plan", {"plan": []})
    assert status == 409 and body["error"]["outcome"] == "not_dispatched"
    assert poll(relay, session) == {"requests": [], "cancelled": ["early"]}
    assert relay.call(session["sessionId"], session["clientToken"], "early", "execute_plan", {"plan": []}) == (status, body)
    with pytest.raises(relay_module.RelayError) as conflict:
        relay.call(session["sessionId"], session["clientToken"], "early", "execute_plan", {"plan": [1]})
    assert conflict.value.body["error"]["code"] == "id_conflict"
    monkeypatch.setattr(relay_module, "MAX_HISTORY", 1)
    with pytest.raises(relay_module.RelayError) as full:
        relay.cancel(session["sessionId"], session["clientToken"], "another")
    assert full.value.status == 429


def test_reply_rejection_preserves_pending_call_and_receipts_are_byte_bounded(mailbox, monkeypatch):
    relay, _ = mailbox
    monkeypatch.setattr(relay_module, "MAX_RESULT_BYTES", relay_module.MAX_REPLY_BYTES)
    session = relay.register("one")
    future = start_call(relay, session, method="execute_plan")
    with pytest.raises(relay_module.RelayError) as undelivered:
        reply(relay, session)
    assert undelivered.value.body["error"]["code"] == "request_not_delivered"
    poll(relay, session)
    with pytest.raises(relay_module.RelayError) as oversized:
        reply(relay, session, value="x" * relay_module.MAX_REPLY_BYTES)
    assert oversized.value.status == 413 and not future.done()
    reply(relay, session, value={"acknowledged": True})
    assert future.result(2) == (200, {"result": {"acknowledged": True}})
    with pytest.raises(relay_module.RelayError) as full:
        relay.call(session["sessionId"], session["clientToken"], "another", "execute_plan", {})
    assert full.value.body["error"]["code"] == "history_limit"


def test_loopback_policy_rejects_remote_peers_and_ambiguous_headers():
    def check(peer="127.0.0.1", rows=()):
        headers = Message()
        headers["Host"] = "localhost:8080"
        for key, value in rows:
            headers[key] = value
        return relay_module._same_origin_loopback(SimpleNamespace(
            client_address=(peer, 5000), headers=headers, server=SimpleNamespace(server_port=8080)))
    assert check(rows=[("Origin", "http://localhost:8080"), ("Sec-Fetch-Site", "same-origin")])
    assert not check(peer="192.0.2.5")
    assert not check(rows=[("Host", "localhost:8080")])
    assert not check(rows=[("Origin", "http://localhost:8080")] * 2)
    assert not check(rows=[("Sec-Fetch-Site", "same-origin")] * 2)
    assert not check(rows=[("Origin", "http://127.0.0.1:8080")])


def test_heartbeat_expiry_and_explicit_disconnect_release_pending_calls_without_replay(mailbox):
    relay, clock = mailbox
    for expire in (False, True):
        session = relay.register("one")
        delivered = start_call(relay, session, "delivered")
        poll(relay, session)
        queued = start_call(relay, session, "queued")
        if expire:
            clock.advance(46)
            relay.wake()
        else:
            relay.disconnect(session["sessionId"], session["browserToken"])
        assert delivered.result(2)[1]["error"]["outcome"] == "unknown"
        assert queued.result(2)[1]["error"]["outcome"] == "not_dispatched"
        assert session["sessionId"] not in relay.sessions
        with pytest.raises(relay_module.RelayError) as error:
            relay.call(session["sessionId"], session["clientToken"], "delivered", "observe", {})
        assert error.value.status == 410


def test_request_deadline_is_unix_ms_and_timed_out_delivery_is_not_retried(mailbox):
    relay, clock = mailbox
    relay.call_seconds = 10
    session = relay.register("one")
    future = start_call(relay, session)
    message = poll(relay, session)["requests"][0]
    assert message["deadline"] == int((clock.wall() + 10) * 1000)
    clock.advance(11)
    relay.wake()
    status, body = future.result(2)
    assert status == 504 and body["error"]["outcome"] == "unknown"
    assert relay.call(session["sessionId"], session["clientToken"], "one", "observe", {}) == (status, body)
    assert poll(relay, session) == {"requests": [], "cancelled": ["one"]}


def test_long_poll_timeout_and_heartbeat_use_controlled_clock(mailbox):
    relay, clock = mailbox
    relay.poll_seconds = 20
    session = relay.register("one")
    result = Future()
    threading.Thread(target=lambda: result.set_result(poll(relay, session)), daemon=True).start()
    with relay.condition:
        assert relay.condition.wait_for(lambda: relay.sessions[session["sessionId"]].polling, 2)
    clock.advance(20)
    relay.wake()
    assert result.result(2) == {"requests": [], "cancelled": []}
    clock.advance(30)
    assert relay.register("new") and session["sessionId"] in relay.sessions
    clock.advance(16)
    relay.register("another")
    assert session["sessionId"] not in relay.sessions


def test_pending_history_and_session_counts_are_bounded(mailbox, monkeypatch):
    relay, clock = mailbox
    sessions = [relay.register(str(i)) for i in range(8)]
    with pytest.raises(relay_module.RelayError) as full:
        relay.register("ninth")
    assert full.value.status == 429
    session = sessions[0]
    waiting = [start_call(relay, session, str(i), method="execute_plan") for i in range(8)]
    with pytest.raises(relay_module.RelayError) as pending:
        relay.call(session["sessionId"], session["clientToken"], "ninth", "observe", {})
    assert pending.value.status == 429
    for i, future in enumerate(waiting):
        relay.cancel(session["sessionId"], session["clientToken"], str(i))
        assert future.result(2)[0] == 409
    monkeypatch.setattr(relay_module, "MAX_HISTORY", 8)
    with pytest.raises(relay_module.RelayError) as history:
        relay.call(session["sessionId"], session["clientToken"], "new", "execute_plan", {})
    assert history.value.body["error"]["code"] == "history_limit"
    clock.advance(46)
    assert relay.register("fresh") and len(relay.sessions) == 1


def test_long_run_read_polling_uses_bounded_cache_without_consuming_mutation_receipts(mailbox):
    relay, clock = mailbox
    session = relay.register("long run")
    write = start_call(relay, session, "start-run", "run_scenario", {"ticks": 100000})
    poll(relay, session)
    reply(relay, session, "start-run", {"runId": "run-1"})
    original = write.result(2)
    relay.cancel(session["sessionId"], session["clientToken"], "early-cancelled-write")
    state = relay.sessions[session["sessionId"]]
    write_bytes = state.result_bytes
    for index in range(360):  # More than 128 requests; 30 minutes at five seconds.
        clock.advance(5)
        identifier = f"status-{index}"
        call = start_call(relay, session, identifier, "run_status", {"runId": "run-1"})
        assert poll(relay, session)["requests"][0]["method"] == "run_status"
        reply(relay, session, identifier, {"tick": index * 250})
        assert call.result(2)[0] == 200
        assert len(state.calls) <= 2 + relay_module.MAX_READ_HISTORY
        assert state.read_result_bytes <= relay_module.MAX_READ_RESULT_BYTES
        assert state.result_bytes == write_bytes
    assert "status-0" not in state.calls and "start-run" in state.calls
    assert clock.value == 2800
    assert state.calls["early-cancelled-write"].fingerprint is None
    cancelled = relay.call(session["sessionId"], session["clientToken"], "early-cancelled-write", "execute_plan", {})
    assert cancelled[1]["error"]["outcome"] == "not_dispatched"
    assert relay.call(session["sessionId"], session["clientToken"], "start-run", "run_scenario", {"ticks": 100000}) == original
    assert poll(relay, session)["requests"] == []
    second = start_call(relay, session, "next-plan", "execute_plan")
    poll(relay, session); reply(relay, session, "next-plan", {"status": "applied"})
    assert second.result(2)[0] == 200


def test_read_receipts_are_byte_bounded_and_do_not_evict_mutation_receipts(mailbox, monkeypatch):
    relay, _ = mailbox
    monkeypatch.setattr(relay_module, "MAX_READ_RESULT_BYTES", 160)
    session = relay.register("bounded reads")
    write = start_call(relay, session, "mutation", "execute_plan")
    poll(relay, session); reply(relay, session, "mutation", {"ack": 1}); result = write.result(2)
    state = relay.sessions[session["sessionId"]]
    for index in range(40):
        identifier = f"read-{index}"
        call = start_call(relay, session, identifier)
        poll(relay, session); reply(relay, session, identifier, "x" * 100)
        assert call.result(2)[0] == 200
        assert state.read_result_bytes <= 160
    assert len(state.calls) == 2 and "mutation" in state.calls
    assert relay.call(session["sessionId"], session["clientToken"], "mutation", "execute_plan", {}) == result


@pytest.mark.parametrize("budget", ["count", "bytes"])
def test_stop_and_read_status_remain_available_when_mutation_history_is_exhausted(mailbox, monkeypatch, budget):
    relay, _ = mailbox
    if budget == "count":
        monkeypatch.setattr(relay_module, "MAX_HISTORY", 1)
    else:
        monkeypatch.setattr(relay_module, "MAX_RESULT_BYTES", relay_module.MAX_REPLY_BYTES)
    session = relay.register("full mutation history")
    write = start_call(relay, session, "mutation", "execute_plan")
    poll(relay, session); reply(relay, session, "mutation", {"ack": 1}); original = write.result(2)
    with pytest.raises(relay_module.RelayError) as full:
        relay.call(session["sessionId"], session["clientToken"], "another", "execute_plan", {})
    assert full.value.body["error"]["code"] == "history_limit"
    status = start_call(relay, session, "status", "run_status", {"runId": "run-1"})
    poll(relay, session); reply(relay, session, "status", {"status": "running"})
    assert status.result(2)[0] == 200
    stop = start_call(relay, session, "urgent-stop", "stop")
    assert poll(relay, session)["requests"][0]["method"] == "stop"
    reply(relay, session, "urgent-stop", {"status": "stopped"})
    assert stop.result(2) == (200, {"result": {"status": "stopped"}})
    assert relay.call(session["sessionId"], session["clientToken"], "mutation", "execute_plan", {}) == original


def test_stop_has_reserved_pending_capacity_priority_and_bounded_flood_coalescing(mailbox):
    relay, _ = mailbox
    session = relay.register("busy")
    waiting = [start_call(relay, session, f"read-{index}") for index in range(relay_module.MAX_PENDING)]
    with pytest.raises(relay_module.RelayError) as full:
        relay.call(session["sessionId"], session["clientToken"], "overflow", "observe", {})
    assert full.value.status == 429
    stop = start_call(relay, session, "stop-first", "stop")
    for index in range(200):
        status, body = relay.call(session["sessionId"], session["clientToken"], f"stop-{index}", "stop", {})
        assert status == 202 and body["result"]["requestId"] == "stop-first"
        assert body["result"]["status"] == "stop_requested"
    state = relay.sessions[session["sessionId"]]
    assert len(state.calls) == relay_module.MAX_PENDING + 1
    assert state.waiters == relay_module.MAX_PENDING
    assert state.stop_waiters == 1
    delivery = poll(relay, session)["requests"]
    assert delivery[0]["id"] == "stop-first"
    reply(relay, session, "stop-first", {"status": "stopped"})
    assert stop.result(2)[0] == 200
    for index, call in enumerate(waiting):
        reply(relay, session, f"read-{index}", {"stopped": True})
        assert call.result(2)[0] == 200
    for index in range(40):
        identifier = f"repeat-stop-{index}"
        current = start_call(relay, session, identifier, "stop")
        poll(relay, session); reply(relay, session, identifier, {"status": "stopped"})
        assert current.result(2)[0] == 200
        assert sum(row.method == "stop" for row in state.calls.values()) == 1


def test_stop_admission_does_not_allocate_another_waiter_while_a_prior_stop_is_returning(mailbox):
    relay, _ = mailbox
    session = relay.register("returning stop")
    state = relay.sessions[session["sessionId"]]
    # Deterministically model the interval between finishing a Stop and its
    # waiting HTTP caller reacquiring the condition to release its slot.
    state.stop_waiters = 1
    status, body = relay.call(session["sessionId"], session["clientToken"], "new-stop", "stop", {})
    assert status == 202 and body["result"]["requestId"] == "new-stop"
    assert body["result"]["coalesced"] is False
    assert state.stop_waiters == 1 and state.waiters == 0
    assert poll(relay, session)["requests"][0]["id"] == "new-stop"
    reply(relay, session, "new-stop", {"status": "stopped"})
    state.stop_waiters = 0
    assert relay.call(session["sessionId"], session["clientToken"], "new-stop", "stop", {})[0] == 200


def test_cancel_notifications_and_stop_cancellation_floods_remain_bounded(mailbox, monkeypatch):
    relay, _ = mailbox
    monkeypatch.setattr(relay_module, "MAX_CANCELLED", 12)
    session = relay.register("cancellation backlog")
    state = relay.sessions[session["sessionId"]]
    for index in range(11):
        identifier = f"read-{index}"
        call = start_call(relay, session, identifier)
        relay.cancel(session["sessionId"], session["clientToken"], identifier)
        assert call.result(2)[0] == 409
    with pytest.raises(relay_module.RelayError) as full:
        relay.call(session["sessionId"], session["clientToken"], "overflow", "observe", {})
    assert full.value.body["error"]["code"] == "cancellation_backlog"
    for index in range(30):
        identifier = f"stop-{index}"
        stop = start_call(relay, session, identifier, "stop")
        relay.cancel(session["sessionId"], session["clientToken"], identifier)
        assert stop.result(2)[0] == 409
        assert len(state.cancelled) == 11
        assert state.stop_cancelled == identifier
        assert sum(row.method == "stop" for row in state.calls.values()) == 1
    delivery = poll(relay, session)
    assert len(delivery["cancelled"]) == 12 and delivery["requests"] == []
    assert state.cancelled == [] and state.stop_cancelled is None


def test_http_rejects_oversized_nonfinite_ambiguous_and_unallowlisted_inputs(local_server, mailbox):
    request = local_server[2]
    session = post(request, "register", {"label": "one"})[2]
    headers = {"Content-Type": "application/json", "Authorization": "Bearer " + session["clientToken"]}
    for method in ("eval", "fetch", [], None):
        assert post(request, "call", {"sessionId": session["sessionId"], "id": "x", "method": method, "args": {}}, headers)[0] == 400
    for body in ('{"label":"one","label":"two"}', '{"label":NaN}', '[]', 'broken'):
        assert request('/api/mcp/register', 'POST', body, BROWSER)[0] == 400
    assert request('/api/mcp/register', 'POST', '{}', {**BROWSER, 'Content-Type': 'text/plain'})[0] == 415
    assert request('/api/mcp/register', 'POST', 'x' * (relay_module.MAX_REQUEST_BYTES + 1), BROWSER)[0] == 413
    assert request('/api/mcp/reply', 'POST', 'x' * (relay_module.MAX_REPLY_BYTES + 1), BROWSER)[0] == 413
    assert not mailbox[0].sessions[session["sessionId"]].calls


def test_http_commands_round_trip_without_cross_session_delivery(local_server, mailbox):
    request = local_server[2]
    one = post(request, "register", {"label": "one"})[2]
    two = post(request, "register", {"label": "two"})[2]
    headers = {"Content-Type": "application/json", "Authorization": "Bearer " + one["clientToken"]}
    result = Future()
    threading.Thread(target=lambda: result.set_result(post(request, "call", {
        "sessionId": one["sessionId"], "id": "from-http", "method": "observe", "args": {}}, headers)), daemon=True).start()
    relay = mailbox[0]
    with relay.condition:
        assert relay.condition.wait_for(lambda: "from-http" in relay.sessions[one["sessionId"]].calls, 2)
    assert post(request, "poll", {"sessionId": two["sessionId"], "browserToken": two["browserToken"]})[2]["requests"] == []
    message = post(request, "poll", {"sessionId": one["sessionId"], "browserToken": one["browserToken"]})[2]
    assert message["requests"][0]["id"] == "from-http"
    assert post(request, "reply", {"sessionId": one["sessionId"], "browserToken": one["browserToken"],
                                   "id": "from-http", "result": {"ownerId": "owner"}})[0] == 200
    assert result.result(2)[2] == {"result": {"ownerId": "owner"}}
    assert post(request, "disconnect", {"sessionId": one["sessionId"], "browserToken": one["browserToken"]})[0] == 200
    assert post(request, "poll", {"sessionId": one["sessionId"], "browserToken": one["browserToken"]})[0] == 410


def test_relay_never_logs_tokens_bodies_or_rejected_url_credentials(local_server, mailbox, monkeypatch, capsys):
    monkeypatch.setattr(serve, "QUIET", False)
    request = local_server[2]
    session = post(request, "register", {"label": "private-label"})[2]
    assert post(request, "poll", {"sessionId": session["sessionId"], "browserToken": session["browserToken"]})[0] == 200
    assert request('/api/mcp/register?clientToken=private-url-token', 'POST', '{"label":"private-body"}', BROWSER)[0] == 400
    output = capsys.readouterr()
    assert not output.out and not output.err
