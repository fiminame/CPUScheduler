#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#pragma warning(disable:4996)

#define INTERRUPT_COUNT 5 // I/O Interrupt되는 횟수.
#define TIME_QUANTUM 5 // Round Robin의 Time Quantum

#define FCFS 1
#define NON_PREEMPTIVE_SJF 2
#define PREEMPTIVE_SJF 3
#define NON_PREEMPTIVE_PRIORITY 4
#define PREEMPTIVE_PRIORITY 5
#define ROUND_ROBIN 6

#define TRUE 1
#define FALSE 0

typedef struct process{ // Process 구조체. Process ID와 도착시간, CPU와 IO Burst 시간, Priority가 들어있다.
	int pid;
	int arrival_time;
	int CPU_burst_time;
	int IO_burst_time;
	int priority;
} process;

typedef struct processNode{ // ProcessNode 구조체. Process를 담는 그릇으로 Ready Queue등에 사용될 것이다.
	process* process_pointer;
	struct processNode* next;
} processNode;

typedef struct GanttNode{ // GanttNode 구조체. GanttChart를 그리는데에 사용될 것이다.
	int pid;
	int time;
	struct GanttNode* next;
} GanttNode;

int evaluateMode[6]; // 평가를 위한 배열1
float evaluateTime[6][2]; // 평가를 위한 배열2

void initProcess(processNode* queue) { // 맨 처음 프로그램을 실행했을 때 임의 개수의 임ㅤㅁㅢㅤ Process를 만들어 Queue에 넣는 작업을 실시하는 함수이다.
	static int processID = 1;

	process* p_pointer = (process*)malloc(sizeof(process));
	p_pointer->pid = processID;
	processID++;
	p_pointer->arrival_time = rand() % 10;
	p_pointer->CPU_burst_time = rand() % 10 + 10;
	p_pointer->IO_burst_time = rand() % 5 + 1;
	p_pointer->priority = rand() % 20;

	while (queue->next != NULL) {
		queue = queue->next;
	}

	processNode* rear = (processNode*)malloc(sizeof(processNode));
	rear->process_pointer = p_pointer;
	rear->next = NULL;

	queue->next = rear;

	return;
}

void createProcess(processNode* queue, process* pointer) { // Queue의 맨 뒤에 processNode* 데이터를 하나 추가해준다.
	process* p_pointer = (process*)malloc(sizeof(process));
	p_pointer->pid = pointer->pid;
	p_pointer->arrival_time = pointer->arrival_time;
	p_pointer->CPU_burst_time = pointer->CPU_burst_time;
	p_pointer->IO_burst_time = pointer->IO_burst_time;
	p_pointer->priority = pointer->priority;

	while (queue->next != NULL) {
		queue = queue->next;
	}

	processNode* rear = (processNode*)malloc(sizeof(processNode));
	rear->process_pointer = p_pointer;
	rear->next = NULL;

	queue->next = rear;

	return;
}

void deleteProcess(processNode* queue, int pid) { // pid를 가지고 Queue안의 데이터를 삭제한다. pid는 unique하기 때문에 pid로 정하였다.
	processNode* temp = queue;
	processNode* prev = queue;
	while (TRUE) {
		prev = temp;
		temp = temp->next;
		if (temp == NULL) return;
		if (temp->process_pointer->pid == pid) {
			prev->next = temp->next;
			return;
		}
	}
}

void printProcess(processNode* queue) { // initProcess를 실행한 후 만들어진 Process들을 보고 싶을 때 실행되는 함수.
	printf("+-------+----------------+------------------+------------------+------------+\n");
	printf("|  PID  |  Arrival Time  |  CPU Burst Time  |  I/O Burst Time  |  Priority  |\n");
	while (TRUE) {
		queue = queue->next;
		if (queue == NULL) break;
		printf("+-------+----------------+------------------+------------------+------------+\n");
		printf("|   %d   |", queue->process_pointer->pid);
		printf("        %d       |", queue->process_pointer->arrival_time);
		printf("        %2d        |", queue->process_pointer->CPU_burst_time);
		printf("         %d        |", queue->process_pointer->IO_burst_time);
		printf("     %2d     |\n", queue->process_pointer->priority);
	}
	printf("+-------+----------------+------------------+------------------+------------+\n");

	return;
}

int* makeIOInterrupt(processNode* queue) { // I/O Interrupt가 언제 실행되는지 정해주는 함수. 미리 정해놓는 이유는 각 Scheduler마다 다른 interrupt가 발생하면 비교에 좋지 않을 것이라 생각하였다.
	int burst_sum = 0;
	int valid = 1;
	while (TRUE) {
		queue = queue->next;
		if (queue == NULL) break;
		burst_sum += queue->process_pointer->CPU_burst_time;
	}

	static int interrupt[INTERRUPT_COUNT];
	for (int i = 0; i < INTERRUPT_COUNT; i++) {
		interrupt[i] = rand() % (burst_sum - 10) + 10;
	}

	while (valid == 1) {
		for (int i = 0; i < INTERRUPT_COUNT; i++) {
			for (int j = 0; j < i; j++) {
				if (interrupt[j] > interrupt[i]) {
					int temp = interrupt[i];
					interrupt[i] = interrupt[j];
					interrupt[j] = temp;
				}
			}
		}
		valid = 0;

		for (int i = 0; i < INTERRUPT_COUNT - 1; i++) {
			if (interrupt[i] == interrupt[i + 1]) {
				interrupt[i + 1] = rand() % (burst_sum - 10) + 10;
				valid = 1;
			}
		}
	}

	return interrupt; // 끝나고나면 interrupt는 크기 INTERRUPT_COUNT만큼의 정렬된 배열이 된다.
}

void printGanttChart(GanttNode* gantt) { // Ganttchart를 display해주는 함수. Scheduling이 끝난 후 자동으로 한번 실행된다.
	printf("\n\nGantt Chart For Scheduling\n\n");
	GanttNode* temp = gantt;
	while (temp->next != NULL) {
		printf("+");
		temp = temp->next;
		if (temp->time < 2) printf("--");
		else {
			for (int i = 0; i < temp->time; i++) printf("-");
		}
	}
	printf("+\n");

	temp = gantt;
	while (temp->next != NULL) {
		printf("|");
		temp = temp->next;
		if (temp->time <= 2) {
			if (temp->pid == 0) printf("  ");
			else printf("P%d", temp->pid);
		}
		else {
			for (int i = 0; i < temp->time / 2 - 1; i++) printf(" ");
			if (temp->pid == 0) printf("  ");
			else printf("P%d", temp->pid);
			for (int i = temp->time / 2 + 1; i < temp->time; i++) printf(" ");
		}
	}
	printf("|\n");

	temp = gantt;
	while (temp->next != NULL) {
		printf("+");
		temp = temp->next;
		if (temp->time < 2) printf("--");
		else {
			for (int i = 0; i < temp->time; i++) printf("-");
		}
	}
	printf("+\n");

	temp = gantt;
	int time = 0;
	while (temp->next != NULL) {
		if (time < 10) printf("%d", time);
		else if (time < 100) printf("%2d", time);
		else printf("%3d", time);
		temp = temp->next;
		time += temp->time;
		if (temp->time < 2) {
			if (time < 10) printf("  ");
			else printf(" ");
		}
		else {
			if (time < 10) {
				for (int i = 0; i < temp->time; i++) printf(" ");
			}
			else {
				for (int i = 0; i < temp->time - 1; i++) printf(" ");
			}
		}
	}
	if (time < 10) printf("%d\n\n", time);
	else if (time < 100) printf("%2d\n\n", time);
	else printf("%3d\n\n", time);
}

void CPUScheduling(processNode* queue, int* IOinterrupt, int mode) { // CPU Scheduling을 실시하는 실질적인 함수.
	int burst_sum = 0; // Scheduling이 진행되는 총 시간을 알기 위하여 만든 변수이다.
	processNode* temp_queue = queue;

	while (TRUE) {
		temp_queue = temp_queue->next;
		if (temp_queue == NULL) break;
		burst_sum += temp_queue->process_pointer->CPU_burst_time; // 각 Process의 CPU Burst Time을 모두 더해준다.
	}

	int time = 0;

	processNode* ready_queue = (processNode*)malloc(sizeof(processNode)); // Ready Queue
	ready_queue->next = NULL;
	processNode* waiting_queue = (processNode*)malloc(sizeof(processNode)); // Waiting Queue
	waiting_queue->next = NULL;
	process* runningProcess = NULL; // 현재 Process
	GanttNode* gantt = (GanttNode*)malloc(sizeof(GanttNode)); // For Ganttchart
	gantt->next = NULL;
	int interrupt_index = 0; // Interrupt를 배열 형식으로 저장해뒀기 때문에 index를 따라가면서 time과 일치하면 interrupt가 발생하도록 하였다.
	int terminated_process_count = 0; // 종료된 process의 개수. 실질적으로 AWT와 ATT를 계산하기 위해 쓰인다.
	int time_quantum_count = 0; // Round Robin에서 사용되는 초세기 변수.

	float average_waiting_time = 0;
	float average_turnaround_time = 0;

	while (time <= burst_sum) {
		if (waiting_queue->next != NULL) { // Waiting Queue안의 Process들을 I/O Burst Time이 끝나면 꺼내어 Ready Queue에 반환시킨다.
			temp_queue = waiting_queue;
			while (TRUE) {
				temp_queue = temp_queue->next;
				if (temp_queue == NULL) break;
				temp_queue->process_pointer->IO_burst_time--; // 여기서 IO Burst Time을 건드리기 때문에
				if (temp_queue->process_pointer->IO_burst_time == 0) {
					process* tempProcess = temp_queue->process_pointer;
					processNode* temp_IO_queue = queue; // Queue에서 같은 pid를 가진 데이터를 가져와서
					while (TRUE) {
						temp_IO_queue = temp_IO_queue->next;
						if (temp_IO_queue == NULL) break;
						if (temp_IO_queue->process_pointer->pid == temp_queue->process_pointer->pid) {
							tempProcess->IO_burst_time = temp_IO_queue->process_pointer->IO_burst_time; // IO Burst Time을 초기화해준다.
							break;
						}
					}
					createProcess(ready_queue, tempProcess); // Ready Queue에 넣고
					deleteProcess(waiting_queue, tempProcess->pid); // Waiting Queue에서는 삭제.
				}
			}
		}

		if (runningProcess != NULL && runningProcess->CPU_burst_time == 0) { // Process가 실행 완료 되어 CPU Burst Time이 0일때 Terminate시켜준다.
			printf("Time : %d, Process %d Terminated!\n", time, runningProcess->pid);
			processNode* temp_queue = queue;
			int process_burst_time; // 반복문을 돌면서 CPU Burst Time부분을 건드렸기 때문에 Queue에서 가져와준다.
			while (TRUE) {
				temp_queue = temp_queue->next;
				if (temp_queue->process_pointer->pid == runningProcess->pid) {
					process_burst_time = temp_queue->process_pointer->CPU_burst_time; // CPU Burst Time을 가져와서 AWT와 ATT 계산에 사용한다.
					break;
				}
			}

			if (terminated_process_count == 0) { // AWT ATT 계산
				average_turnaround_time = (float)(time - runningProcess->arrival_time);
				average_waiting_time = (float)(time - runningProcess->arrival_time - process_burst_time);
			}
			else {
				average_turnaround_time = (average_turnaround_time * terminated_process_count + time - runningProcess->arrival_time) / (terminated_process_count + 1);
				average_waiting_time = (average_waiting_time * terminated_process_count + time - runningProcess->arrival_time - process_burst_time) / (terminated_process_count + 1);
			}

			terminated_process_count++;
			free(runningProcess);
			runningProcess = NULL;
		}

		if (interrupt_index < INTERRUPT_COUNT && time == IOinterrupt[interrupt_index]) { // I/O Interrupt가 발생했을 때 Process를 Waiting Queue로 이동시킨다.
			printf("Time : %d, I/O Interrupted!\n", time);
			interrupt_index++; 
			if (runningProcess != NULL) {
				createProcess(waiting_queue, runningProcess);
				runningProcess = NULL;
			}
		}

		temp_queue = queue; // Scheduling을 시작했을 때 Process가 Arrived되는 시간을 파악해 그 시간이 되면 Process를 Ready Queue에 넣어준다.
		while (TRUE) {
			temp_queue = temp_queue->next;
			if (temp_queue == NULL) break;
			if (temp_queue->process_pointer->arrival_time == time) {
				createProcess(ready_queue, temp_queue->process_pointer);
				printf("Time : %d, Process %d has arrived!\n", time, temp_queue->process_pointer->pid);
			}
		}

		if (runningProcess == NULL && ready_queue->next == NULL) { // 현재 실행중인 Process가 없고 Ready Queue가 비어있을 경우
			if (time >= burst_sum) { // 시간이 끝났으면
				printf("Scheduling End!"); // Scheduling을 종료한다.
				break;
			}
			else { // 그게 아니면 idle하게 기다린다.
				printf("Time : %d, Idle Time\n", time);
				burst_sum++; // idle할 때는 burst_sum을 늘려준다.
			}
		}

		if (runningProcess != NULL || ready_queue->next != NULL) { // 새 Process를 Run하는 부분이다.
			switch (mode) {
			case FCFS:
				/*
				FCFS는 가장 먼저 들어온 Process부터 순차적으로 실행시킨다.
				코드에서는 Ready Queue에서 가장 앞에 있는 Process를 가져온다.
				*/
				if (runningProcess == NULL) {
					runningProcess = ready_queue->next->process_pointer;
					deleteProcess(ready_queue, runningProcess->pid);
				}
				break;
			case NON_PREEMPTIVE_SJF:
				/*
				Non-Preemptive SJF는 가장 작은 Burst Time을 가진 Process부터 순차적으로 실행시킨다.
				코드에서는 Ready Queue를 한번 훑어보고 가장 작은 Burst Time을 가진 Process를 가져온다.
				*/
				if (runningProcess == NULL) {
					temp_queue = ready_queue;
					runningProcess = temp_queue->next->process_pointer;
					while (temp_queue->next != NULL) {
						temp_queue = temp_queue->next;
						runningProcess = (runningProcess->CPU_burst_time > temp_queue->process_pointer->CPU_burst_time) ? temp_queue->process_pointer : runningProcess;
					}
					deleteProcess(ready_queue, runningProcess->pid);
				}
				break;
			case PREEMPTIVE_SJF:
				/*
				Preemptive SJF는 기본적으로 Non-Preemptive SJF와 같으나, 선점형식이라는 부분에서 차이가 있다.
				코드에서는 SJF와 같지만, 선점이 일어날 경우 runningProcess를 교체하고 작업 중이던 Process를 Ready Queue에 다시 넣는다.
				*/
				if (ready_queue->next != NULL) {
					temp_queue = ready_queue;
					process* preemptiveProcess = temp_queue->next->process_pointer;
					while (temp_queue->next != NULL) {
						temp_queue = temp_queue->next;
						preemptiveProcess = (preemptiveProcess->CPU_burst_time > temp_queue->process_pointer->CPU_burst_time) ? temp_queue->process_pointer : preemptiveProcess;
					}
					if (runningProcess == NULL) {
						runningProcess = preemptiveProcess;
						deleteProcess(ready_queue, runningProcess->pid);
					}
					else if (preemptiveProcess->CPU_burst_time < runningProcess->CPU_burst_time) {
						createProcess(ready_queue, runningProcess);
						runningProcess = preemptiveProcess;
						deleteProcess(ready_queue, runningProcess->pid);
					}
				}
				break;
			case NON_PREEMPTIVE_PRIORITY:
				/*
				Non-Preemptive Priority는 가장 낮은 Priority를 가진 Process부터 실행한다.
				코드에서는 Ready Queue를 한번 훑어보고 가장 작은 Priority를 가진 Process를 가져온다.
				*/
				if (runningProcess == NULL) {
					temp_queue = ready_queue;
					runningProcess = temp_queue->next->process_pointer;
					while (temp_queue->next != NULL) {
						temp_queue = temp_queue->next;
						runningProcess = (runningProcess->priority > temp_queue->process_pointer->priority) ? temp_queue->process_pointer : runningProcess;
					}
					deleteProcess(ready_queue, runningProcess->pid);
				}
				break;
			case PREEMPTIVE_PRIORITY:
				/*
				Preemptive Priority는 기본적으로 Non-Preemptive Priority와 같으나, 선점형식이라는 부분에서 차이가 있다.
				코드에서는 Priority와 같지만, 선점이 일어날 경우 runningProcess를 교체하고 작업 중이던 Process를 Ready Queue에 다시 넣는다.
				*/
				if (ready_queue->next != NULL) {
					temp_queue = ready_queue;
					process* preemptiveProcess = temp_queue->next->process_pointer;
					while (temp_queue->next != NULL) {
						temp_queue = temp_queue->next;
						preemptiveProcess = (preemptiveProcess->priority > temp_queue->process_pointer->priority) ? temp_queue->process_pointer : preemptiveProcess;
					}
					if (runningProcess == NULL) {
						runningProcess = preemptiveProcess;
						deleteProcess(ready_queue, runningProcess->pid);
					}
					else if (preemptiveProcess->priority < runningProcess->priority) {
						createProcess(ready_queue, runningProcess);
						runningProcess = preemptiveProcess;
						deleteProcess(ready_queue, runningProcess->pid);
					}
				}
				break;
			case ROUND_ROBIN:
				/*
				Round Robin은 아주 기본적으로는 FCFS의 형식을 따르나, Time Quantum을 넘도록 Process가 실행되면 교체해준다.
				코드에서는 Time Quantum이 넘을 때마다 Ready Queue에서 가장 앞에 있는 Process를 가져오고 작업 중이던 Process를 Ready Queue에 다시 넣는다.
				*/
				if (runningProcess == NULL) {
					runningProcess = ready_queue->next->process_pointer;
					deleteProcess(ready_queue, runningProcess->pid);
					time_quantum_count = 0;
				}
				if (time_quantum_count >= TIME_QUANTUM) {
					createProcess(ready_queue, runningProcess);
					runningProcess = ready_queue->next->process_pointer;
					deleteProcess(ready_queue, runningProcess->pid);
					time_quantum_count = 0;
				}
				break;
			}
			runningProcess->CPU_burst_time--;
			time_quantum_count++;
			printf("Time : %d, Process %d is Executed! Remain Burst Time : %d\n", time, runningProcess->pid, runningProcess->CPU_burst_time);
		}

		GanttNode* temp = gantt; // time이 1 증가할 때마다 GanttChart를 업데이트 시켜준다. gantt도 Queue이다.
		while (temp->next != NULL) {
			temp = temp->next;
		}

		int temppid = (runningProcess == NULL) ? 0 : runningProcess->pid; // O이면 Idle한 상태이고, 그렇지 않은 것들은 실행중인 process의 id이다.

		if (temp == gantt || temp->pid != temppid) { // Queue의 끝부분을 보고 pid가 다르면 새로 하나를 만들어서 넣어준다.
			GanttNode* newGantt = (GanttNode*)malloc(sizeof(GanttNode));
			newGantt->time = 1;
			newGantt->pid = temppid;
			newGantt->next = NULL;
			temp->next = newGantt;
		}
		else { // 아닐경우 시간만 1 증가시켜준다.
			temp->time++;
		}

		time++;
	}

	printGanttChart(gantt);

	printf("Average Waiting Time : %.3lf\n", average_waiting_time);
	printf("Average Turnaround Time : %.3lf\n\n\n", average_turnaround_time);

	evaluateMode[mode - 1] = mode; // mode를 넣은 이유는 평가할 때 정렬을 해야 하는데 AWT와 ATT만 가지고 정렬하다보면 이 Time이 어떤 Scheduler의 것인지 파악하기 힘들기 때문.
	evaluateTime[mode - 1][0] = average_waiting_time;
	evaluateTime[mode - 1][1] = average_turnaround_time;

	return;
}

void evaluateScheduler() {
	for (int i = 0; i < 6; i++) { // Selection 정렬이다.
		int indexMin = i;
		for (int j = i + 1; j < 6; j++) {
			if (evaluateTime[j][1] < evaluateTime[indexMin][1]) indexMin = j;
		}

		int temp = evaluateMode[indexMin];
		evaluateMode[indexMin] = evaluateMode[i];
		evaluateMode[i] = temp;

		float ftemp = evaluateTime[indexMin][0];
		evaluateTime[indexMin][0] = evaluateTime[i][0];
		evaluateTime[i][0] = ftemp;

		ftemp = evaluateTime[indexMin][1];
		evaluateTime[indexMin][1] = evaluateTime[i][1];
		evaluateTime[i][1] = ftemp;
	}
	
	printf("\n\nScheduler Ranking\n\n");
	for (int i = 0; i < 6; i++) {
		printf("%d. ", i + 1);
		switch (evaluateMode[i]) {
		case FCFS:
			printf("FCFS");
			break;
		case NON_PREEMPTIVE_SJF:
			printf("Non-Preemptive SJF");
			break;
		case PREEMPTIVE_SJF:
			printf("Preemptive SJF");
			break;
		case NON_PREEMPTIVE_PRIORITY:
			printf("Non-Preemptive Priority");
			break;
		case PREEMPTIVE_PRIORITY:
			printf("Preemptive Priority");
			break;
		case ROUND_ROBIN:
			printf("Round Robin");
			break;
		}
		printf("\nAverage Waiting Time : %.3lf\nAverage Turnaround Time : %.3lf\n\n", evaluateTime[i][0], evaluateTime[i][1]);
	}
}

int main() {
	processNode* process_queue = (processNode*)malloc(sizeof(processNode)); // Process를 담을 변하지 않는 Queue를 만든다.
	process_queue->next = NULL;
	srand(time(NULL));
	for (int i = 0; i < rand() % 5 + 5; i++) { // Random한 개수의 Process를 만들어준다. 코드에서는 (5 ~ 9)
		initProcess(process_queue);
	}
	int* IOInterrupt;
	IOInterrupt = makeIOInterrupt(process_queue); // I/O Interrupt를 만들어준다.

	char input = NULL;
	while (input != 'c' && input != 'C') {
		printf("------Select Mode------\n1. FCFS\n2. Non-Preemtive SJF\n3. Preemtive SJF\n4. Non-Preemtive Priority\n5. Preemtive Priority\n6. Round Robin\n\n");
		printf("p. Process List\ni. I/O Interrupt Time List\ne. Evaluate Scheduler\nc. End Program\n\nChoose One : ");
		scanf("%c", &input);
		getchar();

		switch (input) {
		case 'p':
		case 'P':
			printProcess(process_queue);
			break;
		case 'i':
		case 'I':
			printf("I/O Interrupt will executing at : ");
			for (int i = 0; i < INTERRUPT_COUNT; i++) {
				printf("%d", IOInterrupt[i]);
				if (i < INTERRUPT_COUNT - 1) printf(", ");
				else printf("\n");
			}
			break;
		case 'e':
		case 'E':
			for (int i = 1; i < 7; i++) CPUScheduling(process_queue, IOInterrupt, i);
			evaluateScheduler();
			break;
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
			CPUScheduling(process_queue, IOInterrupt, input - '0');
			break;
		default:
			break;
		}
	}
}