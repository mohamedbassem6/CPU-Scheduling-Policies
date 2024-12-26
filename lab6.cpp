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

void build_timeline(vector<ProcessTrace>& trace) {
    for (ProcessTrace& pt : trace) {
        for (int i = pt.start_time; i <= pt.end_time; i++) {
            pt.p->timeline[i] = '*';
        }
    }

    for (Process& p : processes) {
        for (int i = p.arrival_time; i < p.finish_time; i++) {
            if (p.timeline[i] == ' ') {
                p.timeline[i] = '.';
            }
        }
    }
}

void print_trace(vector<ProcessTrace>& trace, string policy_name) {
    build_timeline(trace);

    cout << left << setw(6) << policy_name;
    for (int i = 0; i <= total_time; i++) {
        cout << i % 10 << " ";
    }
    cout << endl;

    for (int i = 0; i <= total_time * 2 + 7; i++) {
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

    for (int i = 0; i <= total_time * 2 + 7; i++) {
        cout << "-";
    }
    cout << endl << endl;
}

void print_stats(string policy_name) {
    cout << policy_name << endl;

    cout << "Process    |";
    for (Process& p : processes) {
        cout << "  " << p.name << "  |";
    }
    cout << endl;

    cout << "Arrival    |";
    for (Process& p : processes) {
        cout << right << setw(3) << p.arrival_time << "  |";
    }
    cout << endl;

    cout << "Service    |";
    for (Process& p : processes) {
        cout << right << setw(3) << p.st_p << "  |";
    }
    cout << " Mean|" << endl;

    cout << "Finish     |";
    for (Process& p : processes) {
        cout << right << setw(3) << p.finish_time << "  |";
    }
    cout << "-----|" << endl;

    cout << "Turnaround |";
    double sum = 0;
    for (Process& p : processes) {
        double turnaround = p.finish_time - p.arrival_time;
        cout << right << setw(3) << turnaround << "  |";

        sum += turnaround;
    }
    cout << " " << fixed << setprecision(2) << sum / processes.size() << "|" << endl;

    cout << "NormTurn   |";
    sum = 0;
    for (Process& p : processes) {
        double norm_turn = (p.finish_time - p.arrival_time) / (double)p.st_p;
        cout << " " << fixed << setprecision(2) << norm_turn << "|";

        sum += norm_turn;
    }
    cout << " " << fixed << setprecision(2) << sum / processes.size() << "|" << endl;
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
            current_process->finish_time = fcfs.time;

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
    while (i < processes.size() && processes[i].arrival_time == 0) {
            rr.q.push(&processes[i++]);
    }

    int remaining_quantum = quantum;
    while (rr.time <= total_time) {
        if (rr.q.empty())   break;
        
        Process* current_process = rr.q.front();
        current_process->remaining_time--;
        remaining_quantum--;

        // Preemption
        if (current_process->remaining_time == 0 || remaining_quantum == 0) {
            current_process->finish_time = rr.time + 1;

            ProcessTrace pt(current_process, rr.time - (quantum - remaining_quantum) + 1, rr.time);
            trace.push_back(pt);

            rr.q.pop();
            remaining_quantum = quantum;
        }

        rr.time++;

        // Check if there are any processes that have arrived
        while (i < processes.size() && rr.time == processes[i].arrival_time) {
            rr.q.push(&processes[i++]);
        }

        // Add the current process to the queue if it still has remaining time
        if (current_process->remaining_time > 0) {
            rr.q.push(current_process);
        }
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
        print_trace(process_trace, scheduler_name[scheduler_type]);
    } else {
        print_stats(scheduler_name[scheduler_type]);
    }

    return 0;
}