import os
import re
import sys
import json
import queue
import threading
import subprocess
from http.server import HTTPServer, BaseHTTPRequestHandler
from urllib.parse import urlparse, parse_qs

PORT = 8081
DASHBOARD_DIR = os.path.join(os.path.dirname(__file__), 'dashboard')
BUILD_DIR = os.path.join(os.path.dirname(os.path.dirname(os.path.dirname(__file__))), 'engine', 'build')

tests = {} # {id: {"name": str, "status": "pending|running|passed|failed", "log": list}}
clients = [] # list of queues for SSE
ctest_process = None

def get_test_description(test_name):
    """Attempt to read the first meaningful comment line from the test's .cpp file"""
    import glob
    
    # Try different naming conventions
    possible_names = [
        f"{test_name}.cpp",
        f"test_{test_name}.cpp",
        f"ftd_{test_name}.cpp"
    ]
    
    if test_name.startswith("campaign_"):
        base = test_name.replace("campaign_", "")
        possible_names.extend([f"test_{base}.cpp", f"ftd_{base}.cpp"])
        
    for name in possible_names:
        path = os.path.join(os.path.dirname(BUILD_DIR), "tests", name)
        if os.path.exists(path):
            try:
                with open(path, "r", encoding="utf-8") as f:
                    for _ in range(15): # check first 15 lines
                        line = f.readline().strip()
                        if line.startswith("*") and len(line) > 5 and not "copyright" in line.lower() and not "@file" in line:
                            clean = line.lstrip("* ").strip()
                            if clean.lower().startswith("test:") or clean.lower().startswith("campaign:"):
                                return clean.split(":", 1)[1].strip()
                            if clean.lower().startswith("@brief"):
                                return clean.split("@brief", 1)[1].strip()
                            return clean
            except:
                pass
                
    # Fallback to formatting the name
    return test_name.replace("_", " ").title()

def broadcast_event(event_type, data):
    msg = f"event: {event_type}\ndata: {json.dumps(data)}\n\n"
    for q in clients:
        q.put(msg)

def init_tests():
    global tests
    tests.clear()
    try:
        labels_map = {}
        try:
            j_proc = subprocess.run(["ctest", "-C", "Release", "--show-only=json-v1"], cwd=BUILD_DIR, capture_output=True, text=True)
            j_data = json.loads(j_proc.stdout)
            for t in j_data.get("tests", []):
                t_name = t.get("name")
                for prop in t.get("properties", []):
                    if prop.get("name") == "LABELS":
                        labels_map[t_name] = prop.get("value", [])
        except:
            pass
            
        n_proc = subprocess.run(["ctest", "-N"], cwd=BUILD_DIR, capture_output=True, text=True)
        for line in n_proc.stdout.splitlines():
            m = re.search(r"Test\s+#(\d+):\s+(.+)", line)
            if m:
                t_id = m.group(1)
                t_name = m.group(2).strip()
                t_desc = get_test_description(t_name)
                
                t_labels = labels_map.get(t_name, [])
                category = "Other"
                if "campaign" in t_labels or t_name.startswith("campaign_"):
                    category = "Campaigns"
                elif "gpu" in t_labels or t_name.startswith("gpu_") or t_name.startswith("test_gpu"):
                    category = "GPU Acceleration"
                elif "benchmark" in t_labels or t_name.startswith("benchmark_"):
                    category = "Benchmarks"
                elif "bridge" in t_labels or "web" in t_labels:
                    category = "Engine & Bridge"
                elif "math" in t_labels or "algebra" in t_labels or "clifford" in t_name or "bivector" in t_name:
                    category = "Mathematics"
                elif "unit" in t_labels:
                    category = "Unit Tests"
                else:
                    category = "Unit Tests"
                    
                tests[t_id] = {"name": t_name, "desc": t_desc, "category": category, "status": "pending", "log": []}
    except Exception as e:
        print(f"Error initializing tests: {e}")

def run_ctest():
    global ctest_process
    
    # Reset statuses and logs
    for t in tests.values():
        t["status"] = "pending"
        t["log"] = []
    
    try:
        broadcast_event("init", {"tests": {k: {"name": v["name"], "desc": v["desc"], "category": v["category"], "status": v["status"]} for k, v in tests.items()}})
        
        # 2. Run tests
        ctest_process = subprocess.Popen(
            ["ctest", "-j", "32", "-C", "Release", "-V"],
            cwd=BUILD_DIR,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            bufsize=1
        )
        
        start_re = re.compile(r"^\s*Start\s+(\d+):\s+(.+)$")
        end_re = re.compile(r"Test\s+#(\d+):\s+(.+?)\s+\.*([*]+Failed|\s+Passed)")
        log_re = re.compile(r"^(\d+):\s(.*)$")
        
        for line in ctest_process.stdout:
            line_stripped = line.rstrip()
            
            s_match = start_re.search(line_stripped)
            if s_match:
                t_id = s_match.group(1)
                if t_id in tests:
                    tests[t_id]["status"] = "running"
                    broadcast_event("status", {"id": t_id, "status": "running"})
            
            e_match = end_re.search(line_stripped)
            if e_match:
                t_id = e_match.group(1)
                result = e_match.group(3)
                if t_id in tests:
                    status = "failed" if "Failed" in result else "passed"
                    tests[t_id]["status"] = status
                    broadcast_event("status", {"id": t_id, "status": status})
            
            l_match = log_re.search(line_stripped)
            if l_match:
                t_id = l_match.group(1)
                if t_id in tests:
                    tests[t_id]["log"].append(line_stripped)
                    broadcast_event("log", {"id": t_id, "line": line_stripped})
                    
        ctest_process.wait()
        broadcast_event("done", {})
                    
    except Exception as e:
        print(f"Error running ctest: {e}")
        broadcast_event("error", {"msg": str(e)})

class DashboardHandler(BaseHTTPRequestHandler):
    def log_message(self, format, *args):
        pass # Suppress logging to keep console clean

    def do_GET(self):
        parsed_path = urlparse(self.path)
        path = parsed_path.path
        
        if path == "/":
            path = "/index.html"
            
        if path == "/stream":
            self.send_response(200)
            self.send_header("Content-type", "text/event-stream")
            self.send_header("Cache-Control", "no-cache")
            self.send_header("Connection", "keep-alive")
            self.send_header("Access-Control-Allow-Origin", "*")
            self.end_headers()
            
            q = queue.Queue()
            clients.append(q)
            
            # Send initial state immediately
            init_msg = f"event: init\ndata: {json.dumps({'tests': {k: {'name': v['name'], 'desc': v.get('desc', ''), 'category': v.get('category', 'Other'), 'status': v['status']} for k, v in tests.items()}})}\n\n"
            self.wfile.write(init_msg.encode('utf-8'))
            self.wfile.flush()
            
            try:
                while True:
                    msg = q.get()
                    self.wfile.write(msg.encode('utf-8'))
                    self.wfile.flush()
            except Exception:
                if q in clients:
                    clients.remove(q)
            return

        if path == "/api/logs":
            query = parse_qs(parsed_path.query)
            t_id = query.get("test", [""])[0]
            self.send_response(200)
            self.send_header("Content-type", "application/json")
            self.end_headers()
            log_data = tests.get(t_id, {}).get("log", [])
            self.wfile.write(json.dumps({"log": log_data}).encode('utf-8'))
            return

        # Serve static files
        file_path = os.path.join(DASHBOARD_DIR, path.lstrip('/'))
        if os.path.exists(file_path) and os.path.isfile(file_path):
            self.send_response(200)
            if file_path.endswith('.html'):
                self.send_header("Content-type", "text/html")
            elif file_path.endswith('.css'):
                self.send_header("Content-type", "text/css")
            elif file_path.endswith('.js'):
                self.send_header("Content-type", "application/javascript")
            self.end_headers()
            with open(file_path, 'rb') as f:
                self.wfile.write(f.read())
        else:
            self.send_error(404)

    def do_POST(self):
        if self.path == "/api/start":
            global ctest_process
            if ctest_process is None or ctest_process.poll() is not None:
                threading.Thread(target=run_ctest, daemon=True).start()
                self.send_response(200)
                self.end_headers()
                self.wfile.write(b'{"status": "started"}')
            else:
                self.send_response(400)
                self.end_headers()
                self.wfile.write(b'{"error": "Already running"}')
            return
            
        if self.path == "/api/cancel":
            if ctest_process is not None and ctest_process.poll() is None:
                subprocess.run(["taskkill", "/F", "/T", "/PID", str(ctest_process.pid)])
                self.send_response(200)
                self.end_headers()
                self.wfile.write(b'{"status": "cancelled"}')
            else:
                self.send_response(400)
                self.end_headers()
                self.wfile.write(b'{"error": "Not running"}')
            return

def main():
    init_tests()
    if not os.path.exists(DASHBOARD_DIR):
        os.makedirs(DASHBOARD_DIR)
    server = HTTPServer(('localhost', PORT), DashboardHandler)
    print(f"Dashboard Server started on http://localhost:{PORT}")
    server.serve_forever()

if __name__ == '__main__':
    main()
