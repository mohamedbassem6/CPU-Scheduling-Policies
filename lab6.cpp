#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <queue>
#include <map>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

using namespace std;

typedef struct {
    char name;
    int arrival_time;
    int st_p;
    int remaining_time;
    int finish_time;
    vector<char> timeline;
} Process;

////////////////////////
// Custom Comparators //
////////////////////////
struct ShortestProcessComparator {
    bool operator()(const Process* a, const Process* b) const {
        if (a->st_p == b->st_p) {
            return a->arrival_time > b->arrival_time;
        }
        return a->st_p > b->st_p;
    }
};

struct ShortestRTComparator {
    bool operator()(const Process* a, const Process* b) const {
        if (a->remaining_time == b->remaining_time) {
            return a->arrival_time > b->arrival_time;
        }
        return a->remaining_time > b->remaining_time;
    }
};

struct ArrivalTimeComparator {
    bool operator()(const Process& a, const Process& b) {
        return a.arrival_time < b.arrival_time;
    }
};

//////////////////////
// Global variables //
//////////////////////
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
vector<Process> processes;
int total_time;

//////////////////////
// Output Utilities //
//////////////////////
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
    cout << right << setw(5) << fixed << setprecision(2) << sum / processes.size() << "|" << endl;

    cout << "NormTurn   |";
    sum = 0;
    for (Process& p : processes) {
        double norm_turn = (p.finish_time - p.arrival_time) / (double)p.st_p;
        cout << right << setw(5) << fixed << setprecision(2) << norm_turn << "|";

        sum += norm_turn;
    }
    cout << right << setw(5) << fixed << setprecision(2) << sum / processes.size() << "|" << endl;
    cout << endl;
}

////////////////////////
// Scheduling Policies//
////////////////////////
vector<ProcessTrace> FCFS_Scheduler() {
    int time = 0;

    // Sort processes by arrival time
    sort(processes.begin(), processes.end(), ArrivalTimeComparator());

    vector<ProcessTrace> trace;

    int i = 0;
    while (i < processes.size() && time <= total_time) {
        Process* current_process = &processes[i];

        if (time >= current_process->arrival_time) {
            ProcessTrace pt(current_process, time, time + current_process->st_p - 1);
            trace.push_back(pt);

            time += current_process->remaining_time;
            current_process->remaining_time = 0;
            current_process->finish_time = time;

            i++;
        } else {
            time++;
        }
    }

    return trace;
}

vector<ProcessTrace> RR_Scheduler(int quantum) {
    int time = 0;
    queue<Process*> q;

    vector<ProcessTrace> trace;

    // Sort processes by arrival time
    sort(processes.begin(), processes.end(), ArrivalTimeComparator());

    int i = 0;
    while (i < processes.size() && processes[i].arrival_time == 0) {
            q.push(&processes[i++]);
    }

    int remaining_quantum = quantum;
    while (time <= total_time) {
        if (q.empty())   break;
        
        Process* current_process = q.front();
        current_process->remaining_time--;
        remaining_quantum--;

        // Preemption
        bool preempted = false;
        if (current_process->remaining_time == 0 || remaining_quantum == 0) {
            current_process->finish_time = time + 1;

            ProcessTrace pt(current_process, time - (quantum - remaining_quantum) + 1, time);
            trace.push_back(pt);

            q.pop();
            remaining_quantum = quantum;
            preempted = true;
        }

        time++;

        // Check if there are any processes that have arrived
        while (i < processes.size() && time == processes[i].arrival_time) {
            q.push(&processes[i++]);
        }

        // Add the current process to the queue if it still has remaining time
        if (preempted && current_process->remaining_time > 0) {
            q.push(current_process);
        }
    }

    return trace;
}

vector<ProcessTrace> SPN_Scheduler() {
    vector<ProcessTrace> trace;

    priority_queue<Process*, vector<Process*>, ShortestProcessComparator> pq;

    // Sort processes by arrival time
    sort(processes.begin(), processes.end(), ArrivalTimeComparator());

    int time = 0;
    int i = 0;
    while (time <= total_time) {
        while (i < processes.size() && processes[i].arrival_time <= time) {
            pq.push(&processes[i++]);
        }

        if (pq.empty()) break;

        Process* current_process = pq.top();
        current_process->remaining_time = 0;
        current_process->finish_time = time + current_process->st_p;

        ProcessTrace pt(current_process, time, current_process->finish_time - 1);
        trace.push_back(pt);

        pq.pop();
        time += current_process->st_p;
    }

    return trace;
}

vector<ProcessTrace> SRT_Scheduler() {
    vector<ProcessTrace> trace;

    priority_queue<Process*, vector<Process*>, ShortestRTComparator> pq;

    // Sort processes by arrival time
    sort(processes.begin(), processes.end(), ArrivalTimeComparator());

    int time = 0;
    int i = 0;
    while (time <= total_time) {
        while (i < processes.size() && processes[i].arrival_time <= time) {
            pq.push(&processes[i++]);
        }

        if (pq.empty()) break;

        Process* current_process = pq.top();
        current_process->remaining_time--;

        ProcessTrace pt(current_process, time, time);
        trace.push_back(pt);

        if (current_process->remaining_time == 0) {
            current_process->finish_time = time + 1;
            pq.pop();
        }

        time++;
    }

    return trace;
}

vector<ProcessTrace> HRRN_Scheduler() {
    vector<ProcessTrace> trace;

    // Sort processes by arrival time
    sort(processes.begin(), processes.end(), ArrivalTimeComparator());

    unordered_set<Process*> ready_processes;
    
    int time = 0;
    int i = 0;
    while (time <= total_time) {
        while (i < processes.size() && processes[i].arrival_time <= time) {
            ready_processes.insert(&processes[i++]);
        }

        if (ready_processes.empty()) break;

        // Get the process with the highest response ratio
        Process* current_process;
        double max_hrr = -1;
        for (Process* p : ready_processes) {
            double hrr = (time - p->arrival_time + p->st_p) / (double)p->st_p;
            if (hrr > max_hrr) {
                max_hrr = hrr;
                current_process = p;
            }
        }
        
        ProcessTrace pt(current_process, time, time + current_process->st_p - 1);
        trace.push_back(pt);

        time += current_process->st_p;

        current_process->finish_time = time;
        current_process->remaining_time = 0;
        ready_processes.erase(current_process);
    }

    return trace;
}

int main() {
    string output_type;
    cin >> output_type;
    bool is_trace = (output_type == "trace");

    int scheduler_type, quantum;
    char dash;
    // cin >> scheduler_type >> dash >> quantum;
    cin >> scheduler_type;

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
            process_trace = SPN_Scheduler();
            break;
        case 4:
            process_trace = SRT_Scheduler();
            break;
        case 5:
            process_trace = HRRN_Scheduler();
            break;
        case 6:
            break;
        case 7:
            break;
        case 8:
            break;
    }

    if (is_trace) {
        print_trace(process_trace, scheduler_name[scheduler_type]);
    } else {
        print_stats(scheduler_name[scheduler_type]);
    }

    return 0;
}