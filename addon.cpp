#include <napi.h>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <queue> 

using namespace std;

struct Process {
	string id;
	int arrivalTime;
	int burstTime;
    int priority = 0;
    int completionTime;
    int turnAroundtime;
    int waitingTime;
    int originalBT = burstTime;
    bool completed = false;
};

struct GanttEntry {
    string id;
    int start;
    int end;
};

vector<GanttEntry> ganttChart;
vector<GanttEntry> ganttChartSJB;
vector<GanttEntry> ganttChartPrior;
vector<GanttEntry> ganttChartPriorPre;
vector<GanttEntry> ganttChartRR;
vector<GanttEntry> ganttChartSRTF;



void FCFS(vector<Process>& arr, double& avgTAT, double& avgWT) {
    ganttChart.resize(arr.size());
    sort(arr.begin(), arr.end(), [](const Process& a, const Process& b) {
    return a.arrivalTime < b.arrivalTime;
    });
     int currentTime = arr[0].arrivalTime;

    ganttChart[0].id = arr[0].id;
    ganttChart[0].start = arr[0].arrivalTime;
    ganttChart[0].end = ganttChart[0].start + arr[0].burstTime;
    arr[0].completionTime = ganttChart[0].end;
    arr[0].turnAroundtime = arr[0].completionTime - arr[0].arrivalTime;
    arr[0].waitingTime = arr[0].turnAroundtime - arr[0].burstTime;

    for (int i = 1; i < arr.size(); i++) {
        ganttChart[i].id = arr[i].id;
        ganttChart[i].start = std::max(ganttChart[i - 1].end, arr[i].arrivalTime);
        ganttChart[i].end = ganttChart[i].start + arr[i].burstTime;
        arr[i].completionTime = ganttChart[i].end;
        arr[i].turnAroundtime = arr[i].completionTime - arr[i].arrivalTime;
        arr[i].waitingTime = arr[i].turnAroundtime - arr[i].burstTime;
    }

    int sumWT = 0, sumTAT = 0;

    for (int i = 0; i < arr.size(); i++) {
        sumTAT += arr[i].turnAroundtime;
        sumWT += arr[i].waitingTime;
    }

    avgTAT = (double)sumTAT / (arr.size());
    avgWT = (double)sumWT / (arr.size());
}

int getMinIndex(const vector<Process>& vec) {
    int minIndex = -1;
    for (int i = 0; i < vec.size(); ++i) {
        if (!vec[i].completed) {
            if (minIndex == -1 ||
                vec[i].burstTime < vec[minIndex].burstTime ||
                (vec[i].burstTime == vec[minIndex].burstTime &&
                    vec[i].arrivalTime < vec[minIndex].arrivalTime)) {
                minIndex = i;
            }
        }
    }

    if (minIndex == -1) {
        throw runtime_error("No uncompleted process found");
    }

    return minIndex;
}

int findProcessIndexById(const vector<Process>& vec, const string& id) {
    for (int i = 0; i < vec.size(); ++i) {
        if (vec[i].id == id) {
            return i;
        }
    }
    return -1; // Not found
}

void SJB(vector<Process>& arr, double& avgTAT, double& avgWT) {
    ganttChartSJB.resize(arr.size());

    int count = 0;
    int currentTime = 0;
    int index = 0;

    while (count < arr.size()) {
        
        vector<Process> temp;
        for (int i = 0; i < arr.size(); i++) {
            if (arr[i].arrivalTime <= currentTime && !arr[i].completed) {
                temp.push_back(arr[i]);
            }
        }
       
        if (!temp.empty() && index != 0) {
            int shortestIndex = getMinIndex(temp);
            ganttChartSJB[index].id = temp[shortestIndex].id;
            ganttChartSJB[index].start = std::max(ganttChartSJB[index - 1].end, temp[shortestIndex].arrivalTime);
            ganttChartSJB[index].end = ganttChartSJB[index].start + temp[shortestIndex].burstTime;
            arr[findProcessIndexById(arr, temp[shortestIndex].id)].completionTime = ganttChartSJB[index].end;
            arr[findProcessIndexById(arr, temp[shortestIndex].id)].turnAroundtime = arr[findProcessIndexById(arr, temp[shortestIndex].id)].completionTime - temp[shortestIndex].arrivalTime;
            arr[findProcessIndexById(arr, temp[shortestIndex].id)].waitingTime = arr[findProcessIndexById(arr, temp[shortestIndex].id)].turnAroundtime - temp[shortestIndex].burstTime;
            arr[findProcessIndexById(arr,temp[shortestIndex].id)].completed = true;
            index++;
            currentTime = arr[findProcessIndexById(arr, temp[shortestIndex].id)].completionTime;
            count++;
        }
        else if (!temp.empty() && index == 0) {
            int firstshortestIndex = getMinIndex(temp);
            ganttChartSJB[index].id = temp[firstshortestIndex].id;
            ganttChartSJB[index].start = temp[firstshortestIndex].arrivalTime;
            ganttChartSJB[index].end = temp[firstshortestIndex].arrivalTime + temp[firstshortestIndex].burstTime;
            arr[findProcessIndexById(arr, temp[firstshortestIndex].id)].completionTime = ganttChartSJB[index].end;
            arr[findProcessIndexById(arr, temp[firstshortestIndex].id)].turnAroundtime = arr[findProcessIndexById(arr, temp[firstshortestIndex].id)].completionTime - temp[firstshortestIndex].arrivalTime;
            arr[findProcessIndexById(arr, temp[firstshortestIndex].id)].waitingTime = arr[findProcessIndexById(arr, temp[firstshortestIndex].id)].turnAroundtime - temp[firstshortestIndex].burstTime;
            arr[findProcessIndexById(arr, temp[firstshortestIndex].id)].completed = true;
            index++;
            currentTime = arr[findProcessIndexById(arr, temp[firstshortestIndex].id)].completionTime;
            count++;
        }
        
        if (temp.empty()) {
            currentTime++;
        }
    }

    int sumWT = 0, sumTAT = 0;

    for (int i = 0; i < arr.size(); i++) {
        sumTAT += arr[i].turnAroundtime;
        sumWT += arr[i].waitingTime;
    }

    avgTAT = (double)sumTAT / (arr.size());
    avgWT = (double)sumWT / (arr.size());

}

struct Compare {
    bool operator()(const Process* a, const Process* b) const {
        if (a->priority != b->priority)
            return a->priority > b->priority; // lower number = higher priority
        return a->arrivalTime > b->arrivalTime; // tie-breaker
    }
};

void PriorityNon(vector<Process>& arr, double& avgTAT, double& avgWT) {
    ganttChartPrior.resize(arr.size());
    sort(arr.begin(), arr.end(), [](const Process& a, const Process& b) {
        return a.arrivalTime < b.arrivalTime;
        });
    priority_queue<Process*, vector<Process*>, Compare> queuePrior;
    
    int currentTime = 0;
    int count = 0;
    int index = 0;
    int pointer = 0;

    while (count < arr.size()) {
        while (pointer < arr.size() && arr[pointer].arrivalTime <= currentTime) {
            queuePrior.push(&arr[pointer]);
            pointer++;
        }

        if (!queuePrior.empty() && index == 0) {
            Process* toExecute = queuePrior.top();
            queuePrior.pop();
            ganttChartPrior[index].id = toExecute->id;
            ganttChartPrior[index].start = toExecute->arrivalTime;
            ganttChartPrior[index].end = ganttChartPrior[index].start + toExecute->burstTime;
            toExecute->completionTime = ganttChartPrior[index].end;
            toExecute->turnAroundtime = toExecute->completionTime - toExecute->arrivalTime;
            toExecute->waitingTime = toExecute->turnAroundtime - toExecute->burstTime;
            toExecute->completed = true;
            count++;
            currentTime = toExecute->completionTime;
            index++;
        }
        else if (!queuePrior.empty() && index != 0) {
            Process* toExecute = queuePrior.top();
            queuePrior.pop();
            ganttChartPrior[index].id = toExecute->id;
            ganttChartPrior[index].start = std::max(ganttChartPrior[index - 1].end, toExecute->arrivalTime);
            ganttChartPrior[index].end = ganttChartPrior[index].start + toExecute->burstTime;
            toExecute->completionTime = ganttChartPrior[index].end;
            toExecute->turnAroundtime = toExecute->completionTime - toExecute->arrivalTime;
            toExecute->waitingTime = toExecute->turnAroundtime - toExecute->burstTime;
            toExecute->completed = true;
            count++;
            currentTime = toExecute->completionTime;
            index++;
        }
        else {
            currentTime++;
        }
    }

    int sumWT = 0, sumTAT = 0;

    for (int i = 0; i < arr.size(); i++) {
        sumTAT += arr[i].turnAroundtime;
        sumWT += arr[i].waitingTime;
    }

    avgTAT = (double)sumTAT / (arr.size());
    avgWT = (double)sumWT / (arr.size());
}

void Priority(vector<Process>& arr, double& avgTAT, double& avgWT) {
    ganttChartPriorPre.clear();
    sort(arr.begin(), arr.end(), [](const Process& a, const Process& b) {
        return a.arrivalTime < b.arrivalTime;
        });
    priority_queue<Process*, vector<Process*>, Compare> queuePrior;
    
    int currentTime = 0;
    int count = 0;
    int index = 0;
    int pointer = 0;
    string prevID = "";
    while (count < arr.size()) {
        while (pointer < arr.size() && arr[pointer].arrivalTime <= currentTime) {
            queuePrior.push(&arr[pointer]);
            pointer++;
        }

        if (!queuePrior.empty()) {
            Process* toExecute = queuePrior.top();
            queuePrior.pop();
            if (prevID != toExecute->id) {
                ganttChartPriorPre.push_back({ toExecute->id, currentTime, currentTime + 1 });
            }
            else {
                ganttChartPriorPre.back().end++;   
            }
            toExecute->burstTime--;
            currentTime++;
            prevID = toExecute->id;
            if (toExecute->burstTime != 0) {
                queuePrior.push(toExecute);
            }
            else {
                toExecute->completionTime = currentTime;
                toExecute->turnAroundtime = toExecute->completionTime - toExecute->arrivalTime;
                toExecute->waitingTime = toExecute->turnAroundtime - toExecute->originalBT;
                toExecute->completed = true;
                count++;
            }
        }
        else {
            currentTime++;
            prevID = "";
        }
    }

    int sumWT = 0, sumTAT = 0;

    for (int i = 0; i < arr.size(); i++) {
        sumTAT += arr[i].turnAroundtime;
        sumWT += arr[i].waitingTime;
    }

    avgTAT = (double)sumTAT / (arr.size());
    avgWT = (double)sumWT / (arr.size());

    cout << "\nGantt Chart:\n";
    for (const auto& entry : ganttChartPriorPre) {
        cout << entry.id << ": " << entry.start << " -> " << entry.end << "\n";
    }

    cout << "\nFinal Stats:\n";
    for (const auto& p : arr) {
        cout << p.id << " | CT: " << p.completionTime << ", TAT: " << p.turnAroundtime << ", WT: " << p.waitingTime << "\n";
    }

    cout << "\nAverage TAT: " << avgTAT << "\nAverage WT: " << avgWT << endl;

}

void RoundRobin(vector<Process>& arr, double& avgTAT, double& avgWT, int timeQuantum) {
    ganttChartRR.clear();
    sort(arr.begin(), arr.end(), [](const Process& a, const Process& b) {
        return a.arrivalTime < b.arrivalTime;
        });

    queue<Process*> ProcessQueue;
    int currentTime = 0, count = 0, pointer = 0;

    while (count < arr.size()) {
        while (pointer < arr.size() && arr[pointer].arrivalTime <= currentTime) {
            ProcessQueue.push(&arr[pointer]);
            pointer++;
        }

        if (!ProcessQueue.empty()) {
            Process* toExecute = ProcessQueue.front(); ProcessQueue.pop();

            int start = currentTime;
            int execTime = min(timeQuantum, toExecute->burstTime);
            currentTime += execTime;
            toExecute->burstTime -= execTime;

            ganttChartRR.push_back({ toExecute->id, start, currentTime });

            while (pointer < arr.size() && arr[pointer].arrivalTime <= currentTime) {
                ProcessQueue.push(&arr[pointer]);
                pointer++;
            }

            if (toExecute->burstTime > 0) {
                ProcessQueue.push(toExecute);
            }
            else {
                toExecute->completionTime = currentTime;
                toExecute->turnAroundtime = currentTime - toExecute->arrivalTime;
                toExecute->waitingTime = toExecute->turnAroundtime - toExecute->originalBT;
                toExecute->completed = true;
                count++;
            }
        }
        else {
            currentTime++; 
        }
    }

    int sumWT = 0, sumTAT = 0;
    for (const auto& p : arr) {
        sumWT += p.waitingTime;
        sumTAT += p.turnAroundtime;
    }
    avgWT = (double)sumWT / arr.size();
    avgTAT = (double)sumTAT / arr.size();

    cout << "\nGantt Chart:\n";
    for (const auto& entry : ganttChartRR) {
        cout << entry.id << ": " << entry.start << " -> " << entry.end << "\n";
    }

    cout << "\nFinal Stats:\n";
    for (const auto& p : arr) {
        cout << p.id << " | CT: " << p.completionTime
            << ", TAT: " << p.turnAroundtime
            << ", WT: " << p.waitingTime << "\n";
    }

    cout << "\nAverage TAT: " << avgTAT << "\nAverage WT: " << avgWT << endl;
}

struct CompareSRTF {
    bool operator()(const Process* a, const Process* b) const {
        if (a->burstTime != b->burstTime)
            return a->burstTime > b->burstTime; // Shortest job first
        return a->arrivalTime > b->arrivalTime;
    }
};

void SRTF(vector<Process>& arr, double& avgTAT, double& avgWT) {
    sort(arr.begin(), arr.end(), [](const Process& a, const Process& b) {
        return a.arrivalTime < b.arrivalTime;
        });
    priority_queue<Process*, vector<Process*>, CompareSRTF> queuePrior;

    int currentTime = 0;
    int count = 0;
    int index = 0;
    int pointer = 0;
    string prevID = "";
    while (count < arr.size()) {
        while (pointer < arr.size() && arr[pointer].arrivalTime <= currentTime) {
            queuePrior.push(&arr[pointer]);
            pointer++;
        }

        if (!queuePrior.empty()) {
            Process* toExecute = queuePrior.top();
            queuePrior.pop();
            if (prevID != toExecute->id) {
                ganttChartSRTF.push_back({ toExecute->id, currentTime, currentTime + 1 });
            }
            else {
                ganttChartSRTF.back().end++;
            }
            toExecute->burstTime--;
            currentTime++;
            prevID = toExecute->id;
            if (toExecute->burstTime != 0) {
                queuePrior.push(toExecute);
            }
            else {
                toExecute->completionTime = currentTime;
                toExecute->turnAroundtime = toExecute->completionTime - toExecute->arrivalTime;
                toExecute->waitingTime = toExecute->turnAroundtime - toExecute->originalBT;
                toExecute->completed = true;
                count++;
            }
        }
        else {
            currentTime++;
            prevID = "";
        }
    }

    int sumWT = 0, sumTAT = 0;

    for (int i = 0; i < arr.size(); i++) {
        sumTAT += arr[i].turnAroundtime;
        sumWT += arr[i].waitingTime;
    }

    avgTAT = (double)sumTAT / (arr.size());
    avgWT = (double)sumWT / (arr.size());

    cout << "\nGantt Chart:\n";
    for (const auto& entry : ganttChartSRTF) {
        cout << entry.id << ": " << entry.start << " -> " << entry.end << "\n";
    }

    cout << "\nFinal Stats:\n";
    for (const auto& p : arr) {
        cout << p.id << " | CT: " << p.completionTime << ", TAT: " << p.turnAroundtime << ", WT: " << p.waitingTime << "\n";
    }

    cout << "\nAverage TAT: " << avgTAT << "\nAverage WT: " << avgWT << endl;

}

Napi::Object SRTFWrapped(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    // 1. Get input from JavaScript
    Napi::Array input = info[0].As<Napi::Array>();
    vector<Process> processes;

    for (uint32_t i = 0; i < input.Length(); ++i) {
        Napi::Object obj = input.Get(i).As<Napi::Object>();
        Process p;
        p.id = obj.Get("id").As<Napi::String>();
        p.arrivalTime = obj.Get("arrivalTime").As<Napi::Number>();
        p.burstTime = obj.Get("burstTime").As<Napi::Number>();
        p.originalBT = p.burstTime;
        p.completed = false;
        processes.push_back(p);
    }

    // 2. Call SRTF
    double avgTAT = 0.0, avgWT = 0.0;
    SRTF(processes, avgTAT, avgWT);

    // 3. Build Gantt Chart Array
    Napi::Array ganttArr = Napi::Array::New(env, ganttChartSRTF.size());
    for (size_t i = 0; i < ganttChartSRTF.size(); ++i) {
        Napi::Object entry = Napi::Object::New(env);
        entry.Set("id", ganttChartSRTF[i].id);
        entry.Set("start", ganttChartSRTF[i].start);
        entry.Set("end", ganttChartSRTF[i].end);
        ganttArr.Set(i, entry);
    }

    // 4. Build Process Results Array
    Napi::Array processArr = Napi::Array::New(env, processes.size());
    for (size_t i = 0; i < processes.size(); ++i) {
        const Process& p = processes[i];
        Napi::Object obj = Napi::Object::New(env);
        obj.Set("id", p.id);
        obj.Set("arrivalTime", p.arrivalTime);
        obj.Set("burstTime", p.originalBT);
        obj.Set("completionTime", p.completionTime);
        obj.Set("turnAroundtime", p.turnAroundtime);
        obj.Set("waitingTime", p.waitingTime);
        processArr.Set(i, obj);
    }

    // 5. Build Final Return Object
    Napi::Object result = Napi::Object::New(env);
    result.Set("ganttChart", ganttArr);
    result.Set("processes", processArr);
    result.Set("averageTAT", avgTAT);
    result.Set("averageWT", avgWT);

    return result;
}


Napi::Object RRWrapped(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    // Argument 0 = process array
    // Argument 1 = time quantum
    Napi::Array input = info[0].As<Napi::Array>();
    int timeQuantum = info[1].As<Napi::Number>();

    vector<Process> processes;
    for (uint32_t i = 0; i < input.Length(); ++i) {
        Napi::Object obj = input.Get(i).As<Napi::Object>();
        Process p;
        p.id = obj.Get("id").As<Napi::String>();
        p.arrivalTime = obj.Get("arrivalTime").As<Napi::Number>();
        p.burstTime = obj.Get("burstTime").As<Napi::Number>();
        p.originalBT = p.burstTime;
        p.completed = false;
        processes.push_back(p);
    }

    double avgTAT = 0.0, avgWT = 0.0;
    RoundRobin(processes, avgTAT, avgWT, timeQuantum);

    // Convert gantt chart
    Napi::Array ganttArr = Napi::Array::New(env, ganttChartRR.size());
    for (size_t i = 0; i < ganttChartRR.size(); ++i) {
        Napi::Object obj = Napi::Object::New(env);
        obj.Set("id", ganttChartRR[i].id);
        obj.Set("start", ganttChartRR[i].start);
        obj.Set("end", ganttChartRR[i].end);
        ganttArr.Set(i, obj);
    }

    // Convert process results
    Napi::Array processArr = Napi::Array::New(env, processes.size());
    for (size_t i = 0; i < processes.size(); ++i) {
        const Process& p = processes[i];
        Napi::Object obj = Napi::Object::New(env);
        obj.Set("id", p.id);
        obj.Set("arrivalTime", p.arrivalTime);
        obj.Set("burstTime", p.originalBT); // original burst
        obj.Set("completionTime", p.completionTime);
        obj.Set("turnAroundtime", p.turnAroundtime);
        obj.Set("waitingTime", p.waitingTime);
        processArr.Set(i, obj);
    }

    // Final object to return
    Napi::Object result = Napi::Object::New(env);
    result.Set("ganttChart", ganttArr);
    result.Set("processes", processArr);
    result.Set("averageTAT", avgTAT);
    result.Set("averageWT", avgWT);

    return result;
}


Napi::Object PriorPreWrapped(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    Napi::Array input = info[0].As<Napi::Array>();
    vector<Process> processes;

    for (uint32_t i = 0; i < input.Length(); ++i) {
        Napi::Object obj = input.Get(i).As<Napi::Object>();
        Process p;
        p.id = obj.Get("id").As<Napi::String>();
        p.arrivalTime = obj.Get("arrivalTime").As<Napi::Number>();
        p.burstTime = obj.Get("burstTime").As<Napi::Number>();
        p.originalBT = p.burstTime; // needed for WT/TAT calc
        p.priority = obj.Get("priority").As<Napi::Number>();
        p.completed = false; // initialize completed flag
        processes.push_back(p);
    }

    double avgTAT = 0.0, avgWT = 0.0;
    Priority(processes, avgTAT, avgWT); // Call your preemptive function

    // Convert Gantt chart
    Napi::Array ganttArr = Napi::Array::New(env, ganttChartPriorPre.size());
    for (size_t i = 0; i < ganttChartPriorPre.size(); ++i) {
        const GanttEntry& entry = ganttChartPriorPre[i];
        Napi::Object obj = Napi::Object::New(env);
        obj.Set("id", entry.id);
        obj.Set("start", entry.start);
        obj.Set("end", entry.end);
        ganttArr.Set(i, obj);
    }

    // Convert process results
    Napi::Array processArr = Napi::Array::New(env, processes.size());
    for (size_t i = 0; i < processes.size(); ++i) {
        const Process& p = processes[i];
        Napi::Object obj = Napi::Object::New(env);
        obj.Set("id", p.id);
        obj.Set("arrivalTime", p.arrivalTime);
        obj.Set("burstTime", p.originalBT); // original burst time
        obj.Set("priority", p.priority);
        obj.Set("completionTime", p.completionTime);
        obj.Set("turnAroundtime", p.turnAroundtime);
        obj.Set("waitingTime", p.waitingTime);
        processArr.Set(i, obj);
    }

    // Final object
    Napi::Object result = Napi::Object::New(env);
    result.Set("ganttChart", ganttArr);
    result.Set("processes", processArr);
    result.Set("averageTAT", avgTAT);
    result.Set("averageWT", avgWT);

    return result;
}


Napi::Object PriorWrapped(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    Napi::Array input = info[0].As<Napi::Array>();
    vector<Process> processes;

    for (uint32_t i = 0; i < input.Length(); ++i) {
        Napi::Object obj = input.Get(i).As<Napi::Object>();
        Process p;
        p.id = obj.Get("id").As<Napi::String>();
        p.arrivalTime = obj.Get("arrivalTime").As<Napi::Number>();
        p.burstTime = obj.Get("burstTime").As<Napi::Number>();
        p.priority = obj.Get("priority").As<Napi::Number>();
        processes.push_back(p);
    }

    double avgTAT = 0.0, avgWT = 0.0;
    PriorityNon(processes, avgTAT, avgWT); // Updated call

    // Convert gantt chart
    Napi::Array ganttArr = Napi::Array::New(env, ganttChartPrior.size());
    for (size_t i = 0; i < ganttChartPrior.size(); ++i) {
        Napi::Object obj = Napi::Object::New(env);
        obj.Set("id", ganttChartPrior[i].id);
        obj.Set("start", ganttChartPrior[i].start);
        obj.Set("end", ganttChartPrior[i].end);
        ganttArr.Set(i, obj);
    }

    // Convert processes
    Napi::Array processArr = Napi::Array::New(env, processes.size());
    for (size_t i = 0; i < processes.size(); ++i) {
        Napi::Object obj = Napi::Object::New(env);
        obj.Set("id", processes[i].id);
        obj.Set("arrivalTime", processes[i].arrivalTime);
        obj.Set("burstTime", processes[i].burstTime);
        obj.Set("completionTime", processes[i].completionTime);
        obj.Set("turnAroundtime", processes[i].turnAroundtime);
        obj.Set("waitingTime", processes[i].waitingTime);
        processArr.Set(i, obj);
    }

    // Final result object
    Napi::Object result = Napi::Object::New(env);
    result.Set("ganttChart", ganttArr);
    result.Set("processes", processArr);
    result.Set("averageTAT", avgTAT);
    result.Set("averageWT", avgWT);

    return result;
}

Napi::Object SJBWrapped(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    Napi::Array input = info[0].As<Napi::Array>();
    vector<Process> processes;

    for (uint32_t i = 0; i < input.Length(); ++i) {
        Napi::Object obj = input.Get(i).As<Napi::Object>();
        Process p;
        p.id = obj.Get("id").As<Napi::String>();
        p.arrivalTime = obj.Get("arrivalTime").As<Napi::Number>();
        p.burstTime = obj.Get("burstTime").As<Napi::Number>();
        processes.push_back(p);
    }

    double avgTAT = 0.0, avgWT = 0.0;
    SJB(processes, avgTAT, avgWT); // Updated call

    // Convert gantt chart
    Napi::Array ganttArr = Napi::Array::New(env, ganttChartSJB.size());
    for (size_t i = 0; i < ganttChartSJB.size(); ++i) {
        Napi::Object obj = Napi::Object::New(env);
        obj.Set("id", ganttChartSJB[i].id);
        obj.Set("start", ganttChartSJB[i].start);
        obj.Set("end", ganttChartSJB[i].end);
        ganttArr.Set(i, obj);
    }

    // Convert processes
    Napi::Array processArr = Napi::Array::New(env, processes.size());
    for (size_t i = 0; i < processes.size(); ++i) {
        Napi::Object obj = Napi::Object::New(env);
        obj.Set("id", processes[i].id);
        obj.Set("arrivalTime", processes[i].arrivalTime);
        obj.Set("burstTime", processes[i].burstTime);
        obj.Set("completionTime", processes[i].completionTime);
        obj.Set("turnAroundtime", processes[i].turnAroundtime);
        obj.Set("waitingTime", processes[i].waitingTime);
        processArr.Set(i, obj);
    }

    // Final result object
    Napi::Object result = Napi::Object::New(env);
    result.Set("ganttChart", ganttArr);
    result.Set("processes", processArr);
    result.Set("averageTAT", avgTAT);
    result.Set("averageWT", avgWT);

    return result;
}

Napi::Object FCFSWrapped(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    Napi::Array input = info[0].As<Napi::Array>();
    vector<Process> processes;

    for (uint32_t i = 0; i < input.Length(); ++i) {
        Napi::Object obj = input.Get(i).As<Napi::Object>();
        Process p;
        p.id = obj.Get("id").As<Napi::String>();
        p.arrivalTime = obj.Get("arrivalTime").As<Napi::Number>();
        p.burstTime = obj.Get("burstTime").As<Napi::Number>();
        processes.push_back(p);
    }

    double avgTAT = 0.0, avgWT = 0.0;
    FCFS(processes, avgTAT, avgWT); // Updated call

    // Convert gantt chart
    Napi::Array ganttArr = Napi::Array::New(env, ganttChart.size());
    for (size_t i = 0; i < ganttChart.size(); ++i) {
        Napi::Object obj = Napi::Object::New(env);
        obj.Set("id", ganttChart[i].id);
        obj.Set("start", ganttChart[i].start);
        obj.Set("end", ganttChart[i].end);
        ganttArr.Set(i, obj);
    }

    // Convert processes
    Napi::Array processArr = Napi::Array::New(env, processes.size());
    for (size_t i = 0; i < processes.size(); ++i) {
        Napi::Object obj = Napi::Object::New(env);
        obj.Set("id", processes[i].id);
        obj.Set("arrivalTime", processes[i].arrivalTime);
        obj.Set("burstTime", processes[i].burstTime);
        obj.Set("completionTime", processes[i].completionTime);
        obj.Set("turnAroundtime", processes[i].turnAroundtime);
        obj.Set("waitingTime", processes[i].waitingTime);
        processArr.Set(i, obj);
    }

    // Final result object
    Napi::Object result = Napi::Object::New(env);
    result.Set("ganttChart", ganttArr);
    result.Set("processes", processArr);
    result.Set("averageTAT", avgTAT);
    result.Set("averageWT", avgWT);

    return result;
}



Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set("fcfs", Napi::Function::New(env, FCFSWrapped));
    exports.Set("sjb", Napi::Function::New(env, SJBWrapped));
    exports.Set("priornon",Napi::Function::New(env,PriorWrapped));
    exports.Set("prior",Napi::Function::New(env,PriorPreWrapped));
    exports.Set("rr", Napi::Function::New(env, RRWrapped));
    exports.Set("srtf", Napi::Function::New(env, SRTFWrapped));
    return exports;
}

NODE_API_MODULE(addon, Init)
