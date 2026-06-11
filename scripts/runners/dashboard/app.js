const grid = document.getElementById('test-grid');
const logPanel = document.getElementById('log-panel');
const logTitle = document.getElementById('log-title');
const logContent = document.getElementById('log-content');
const startBtn = document.getElementById('start-btn');
const cancelBtn = document.getElementById('cancel-btn');
const closeBtn = document.getElementById('close-log-btn');

let currentTestId = null;
let eventSource = null;

// Stats
const stats = { passed: 0, failed: 0, running: 0, pending: 0 };

function updateStatsUI() {
    document.getElementById('stat-passed').innerText = `${stats.passed} Passed`;
    document.getElementById('stat-failed').innerText = `${stats.failed} Failed`;
    document.getElementById('stat-running').innerText = `${stats.running} Running`;
    document.getElementById('stat-pending').innerText = `${stats.pending} Pending`;
}

function createCard(id, test) {
    const card = document.createElement('div');
    card.className = 'test-card';
    card.id = `test-${id}`;
    card.dataset.status = test.status;
    
    card.innerHTML = `
        <div class="test-id">#${id}</div>
        <div class="test-name">${test.name}</div>
        <div class="test-desc" title="${test.desc || ''}">${test.desc || ''}</div>
    `;
    
    card.addEventListener('click', () => openLog(id, test.name));
    return card;
}

async function openLog(id, name) {
    currentTestId = id;
    logTitle.innerText = `Logs: ${name}`;
    logPanel.classList.remove('hidden');
    logContent.innerText = "Loading logs...";
    
    // Fetch historical log
    const res = await fetch(`/api/logs?test=${id}`);
    const data = await res.json();
    logContent.innerText = data.log.join('\n') + '\n';
    scrollToBottom();
}

closeBtn.addEventListener('click', () => {
    logPanel.classList.add('hidden');
    currentTestId = null;
});

function scrollToBottom() {
    logContent.scrollTop = logContent.scrollHeight;
}

function connectSSE() {
    if (eventSource) return;
    eventSource = new EventSource('/stream');
    
    eventSource.addEventListener('init', (e) => {
        const data = JSON.parse(e.data);
        grid.innerHTML = '';
        stats.passed = stats.failed = stats.running = stats.pending = 0;
        
        const sortedKeys = Object.keys(data.tests).sort((a,b) => parseInt(a) - parseInt(b));
        const categoryMap = {};
        
        for (const id of sortedKeys) {
            const test = data.tests[id];
            stats[test.status]++;
            const cat = test.category || "Other";
            if (!categoryMap[cat]) categoryMap[cat] = [];
            categoryMap[cat].push({id, test});
        }
        
        const catOrder = ["Campaigns", "GPU Acceleration", "Benchmarks", "Mathematics", "Engine & Bridge", "Unit Tests", "Other"];
        const cats = Object.keys(categoryMap).sort((a,b) => {
            let ia = catOrder.indexOf(a);
            let ib = catOrder.indexOf(b);
            if (ia === -1) ia = 99;
            if (ib === -1) ib = 99;
            return ia - ib;
        });
        
        for (const cat of cats) {
            const section = document.createElement('div');
            section.className = 'category-section';
            section.innerHTML = `<h2 class="category-title">${cat} <span class="category-count">(${categoryMap[cat].length})</span></h2>`;
            
            const catGrid = document.createElement('div');
            catGrid.className = 'grid-container';
            
            for (const item of categoryMap[cat]) {
                catGrid.appendChild(createCard(item.id, item.test));
            }
            
            section.appendChild(catGrid);
            grid.appendChild(section);
        }
        
        updateStatsUI();
    });
    
    eventSource.addEventListener('status', (e) => {
        const data = JSON.parse(e.data);
        const card = document.getElementById(`test-${data.id}`);
        if (card) {
            const oldStatus = card.dataset.status;
            card.dataset.status = data.status;
            stats[oldStatus]--;
            stats[data.status]++;
            updateStatsUI();
        }
    });
    
    eventSource.addEventListener('log', (e) => {
        const data = JSON.parse(e.data);
        if (currentTestId === data.id) {
            logContent.innerText += data.line + '\n';
            scrollToBottom();
        }
    });
    
    eventSource.addEventListener('done', () => {
        startBtn.disabled = false;
        startBtn.innerText = "Run Suite";
        cancelBtn.style.display = 'none';
    });
}

startBtn.addEventListener('click', async () => {
    startBtn.disabled = true;
    startBtn.innerText = "Starting...";
    cancelBtn.style.display = 'inline-block';
    await fetch('/api/start', { method: 'POST' });
    startBtn.innerText = "Running Suite";
});

cancelBtn.addEventListener('click', async () => {
    cancelBtn.disabled = true;
    cancelBtn.innerText = "Cancelling...";
    await fetch('/api/cancel', { method: 'POST' });
    cancelBtn.innerText = "Cancel All";
    cancelBtn.disabled = false;
});

// Init
connectSSE();
