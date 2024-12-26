#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <queue>
#include <map>

using namespace std;

unordered_map<int, string> scheduler_name = {
    {1, "FCFS"},
    {2, "RR"},
    {3, "SPN"},
    {4, "SRT"},
    {5, "HRRN"},
    {6, "FB-1"},
    {7, "FB-2i"},
    {8, "Aging"}
};

typedef struct {
    char name;
    int arrival_time;
    int st_p;
    int remaining_time;
    int finish_time;
    vector<char> timeline;
} Process;

vector<Process> processes;
int total_time;

struct ProcessTrace {
    Process* p;
    int start_time, end_time;

    // Constructor to initialize p
    ProcessTrace(Process* process, int start, int end)
        : p(process), start_time(start), end_time(end) {}
};

void update_timeline(vector<ProcessTrace>& trace) {
    for (ProcessTrace& pt : trace) {
        for (int i = pt.start_time; i <= pt.end_time; i++) {
            pt.p->timeline[i] = '*';
        }
    }

    for (Process& p : processes) {
        for (int i = p.arrival_time; i <= p.finish_time; i++) {
            if (p.timeline[i] == ' ') {
                p.timeline[i] = '.';
            }
        }
    }
}

void trace(vector<ProcessTrace>& trace, string policy_name) {
    update_timeline(trace);

    cout << left << setw(6) << policy_name;
    for (int i = 0; i <= total_time; i++) {
        cout << i % 10 << " ";
    }
    cout << endl;

    for (int i = 0; i <= total_time * 2 + 6; i++) {
        cout << "-";
    }
    cout << endl;

    for (Process& p : processes) {
        cout << left << setw(6) << p.name;
        for (int i = 0; i < total_time; i++) {
            cout << "|" << p.timeline[i];
        }
        cout << "| " << endl;
    }

    for (int i = 0; i <= total_time * 2 + 6; i++) {
        cout << "-";
    }
    cout << endl;
}

////////////////////////////
// First Come First Serve //
////////////////////////////
typedef struct {
    queue<Process*> q;
    int time;
} FCFS;

vector<ProcessTrace> FCFS_Scheduler() {
    FCFS fcfs;
    fcfs.time = 0;

    vector<ProcessTrace> trace;

    // Sort processes by arrival time
    sort(processes.begin(), processes.end(), [](Process a, Process b) {
        return a.arrival_time < b.arrival_time;
    });

    // Push pointers to processes into the queue
    for (Process& process : processes) {
        fcfs.q.push(&process);
    }

    while (fcfs.time <= total_time) {
        if (fcfs.q.empty())
            break;

        Process* current_process = fcfs.q.front();

        if (fcfs.time >= current_process->arrival_time) {
            ProcessTrace pt(current_process, fcfs.time, fcfs.time + current_process->st_p - 1);
            trace.push_back(pt);

            fcfs.time += current_process->remaining_time;
            current_process->remaining_time = 0;
            current_process->finish_time = pt.end_time;

            fcfs.q.pop();
        } else {
            fcfs.time++;
        }
    }
    return trace;
}

/////////////////
// Round Robin //
/////////////////
typedef struct {
    queue<Process*> q;
    int time;
} RR;

vector<ProcessTrace> RR_Scheduler(int quantum) {
    RR rr;
    rr.time = 0;

    vector<ProcessTrace> trace;

    // Sort processes by arrival time
    sort(processes.begin(), processes.end(), [](Process a, Process b) {
        return a.arrival_time < b.arrival_time;
    });

    int i = 0;
    rr.q.push(&processes[i++]);
    while (rr.time <= total_time) {
        while (i < processes.size() && processes[i].arrival_time <= rr.time) {
            rr.q.push(&processes[i]);
            i++;
        }

        if (rr.q.empty())   break;
        
        Process* current_process = rr.q.front();
        int exec_time = min(quantum, current_process->remaining_time);

        ProcessTrace pt(current_process, rr.time, rr.time + exec_time - 1);
        trace.push_back(pt);

        current_process->remaining_time -= exec_time;

        if (current_process->remaining_time == 0) {
            current_process->finish_time = rr.time;
            rr.q.pop();
        } else {    // If the process is not finished, push it to the back of the queue
            rr.q.pop();
            rr.q.push(current_process);
        }
        
        rr.time += exec_time;
    }

    return trace;
}

int main() {
    string output_type;
    cin >> output_type;
    bool is_trace = (output_type == "trace");

    int scheduler_type, quantum;
    char dash;
    cin >> scheduler_type >> dash >> quantum;
    // cin >> scheduler_type;

    cin >> total_time;

    int processes_count;
    cin >> processes_count;

    processes = vector<Process>(processes_count);
    cin.ignore();
    for (int i = 0; i < processes_count; i++) {
        string line;
        getline(cin, line);
        stringstream ss(line);

        char comma;
        ss >> processes[i].name >> comma
           >> processes[i].arrival_time >> comma
           >> processes[i].st_p;

        processes[i].remaining_time = processes[i].st_p;
        processes[i].finish_time = -1;
        processes[i].timeline = vector<char>(total_time, ' ');
    }

    vector<ProcessTrace> process_trace;
    switch (scheduler_type) {
        case 1:
            process_trace = FCFS_Scheduler();
            break;
        case 2:
            process_trace = RR_Scheduler(quantum);
            break;
        case 3:
            break;
    }

    if (is_trace) {
        trace(process_trace, scheduler_name[scheduler_type]);
    }

    return 0;
}