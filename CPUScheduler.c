#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#pragma warning(disable:4996)

#define INTERRUPT_COUNT 5 // I/O Interrupt�Ǵ� Ƚ��.
#define TIME_QUANTUM 5 // Round Robin�� Time Quantum

#define FCFS 1
#define NON_PREEMPTIVE_SJF 2
#define PREEMPTIVE_SJF 3
#define NON_PREEMPTIVE_PRIORITY 4
#define PREEMPTIVE_PRIORITY 5
#define ROUND_ROBIN 6

#define TRUE 1
#define FALSE 0

typedef struct process{ // Process ����ü. Process ID�� �����ð�, CPU�� IO Burst �ð�, Priority�� ����ִ�.
	int pid;
	int arrival_time;
	int CPU_burst_time;
	int IO_burst_time;
	int priority;
} process;

typedef struct processNode{ // ProcessNode ����ü. Process�� ��� �׸����� Ready Queue� ���� ���̴�.
	process* process_pointer;
	struct processNode* next;
} processNode;

typedef struct GanttNode{ // GanttNode ����ü. GanttChart�� �׸��µ��� ���� ���̴�.
	int pid;
	int time;
	struct GanttNode* next;
} GanttNode;

int evaluateMode[6]; // �򰡸� ���� �迭1
float evaluateTime[6][2]; // �򰡸� ���� �迭2

void initProcess(processNode* queue) { // �� ó�� ���α׷��� �������� �� ���� ������ �ӤԤ��Ҥ� Process�� ����� Queue�� �ִ� �۾��� �ǽ��ϴ� �Լ��̴�.
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

void createProcess(processNode* queue, process* pointer) { // Queue�� �� �ڿ� processNode* �����͸� �ϳ� �߰����ش�.
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

void deleteProcess(processNode* queue, int pid) { // pid�� ������ Queue���� �����͸� �����Ѵ�. pid�� unique�ϱ� ������ pid�� ���Ͽ���.
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

void printProcess(processNode* queue) { // initProcess�� ������ �� ������� Process���� ���� ���� �� ����Ǵ� �Լ�.
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

int* makeIOInterrupt(processNode* queue) { // I/O Interrupt�� ���� ����Ǵ��� �����ִ� �Լ�. �̸� ���س��� ������ �� Scheduler���� �ٸ� interrupt�� �߻��ϸ� �񱳿� ���� ���� ���̶� �����Ͽ���.
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

	return interrupt; // �������� interrupt�� ũ�� INTERRUPT_COUNT��ŭ�� ���ĵ� �迭�� �ȴ�.
}

void printGanttChart(GanttNode* gantt) { // Ganttchart�� display���ִ� �Լ�. Scheduling�� ���� �� �ڵ����� �ѹ� ����ȴ�.
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

void CPUScheduling(processNode* queue, int* IOinterrupt, int mode) { // CPU Scheduling�� �ǽ��ϴ� �������� �Լ�.
	int burst_sum = 0; // Scheduling�� ����Ǵ� �� �ð��� �˱� ���Ͽ� ���� �����̴�.
	processNode* temp_queue = queue;

	while (TRUE) {
		temp_queue = temp_queue->next;
		if (temp_queue == NULL) break;
		burst_sum += temp_queue->process_pointer->CPU_burst_time; // �� Process�� CPU Burst Time�� ��� �����ش�.
	}

	int time = 0;

	processNode* ready_queue = (processNode*)malloc(sizeof(processNode)); // Ready Queue
	ready_queue->next = NULL;
	processNode* waiting_queue = (processNode*)malloc(sizeof(processNode)); // Waiting Queue
	waiting_queue->next = NULL;
	process* runningProcess = NULL; // ���� Process
	GanttNode* gantt = (GanttNode*)malloc(sizeof(GanttNode)); // For Ganttchart
	gantt->next = NULL;
	int interrupt_index = 0; // Interrupt�� �迭 �������� �����صױ� ������ index�� ���󰡸鼭 time�� ��ġ�ϸ� interrupt�� �߻��ϵ��� �Ͽ���.
	int terminated_process_count = 0; // ����� process�� ����. ���������� AWT�� ATT�� ����ϱ� ���� ���δ�.
	int time_quantum_count = 0; // Round Robin���� ���Ǵ� �ʼ��� ����.

	float average_waiting_time = 0;
	float average_turnaround_time = 0;

	while (time <= burst_sum) {
		if (waiting_queue->next != NULL) { // Waiting Queue���� Process���� I/O Burst Time�� ������ ������ Ready Queue�� ��ȯ��Ų��.
			temp_queue = waiting_queue;
			while (TRUE) {
				temp_queue = temp_queue->next;
				if (temp_queue == NULL) break;
				temp_queue->process_pointer->IO_burst_time--; // ���⼭ IO Burst Time�� �ǵ帮�� ������
				if (temp_queue->process_pointer->IO_burst_time == 0) {
					process* tempProcess = temp_queue->process_pointer;
					processNode* temp_IO_queue = queue; // Queue���� ���� pid�� ���� �����͸� �����ͼ�
					while (TRUE) {
						temp_IO_queue = temp_IO_queue->next;
						if (temp_IO_queue == NULL) break;
						if (temp_IO_queue->process_pointer->pid == temp_queue->process_pointer->pid) {
							tempProcess->IO_burst_time = temp_IO_queue->process_pointer->IO_burst_time; // IO Burst Time�� �ʱ�ȭ���ش�.
							break;
						}
					}
					createProcess(ready_queue, tempProcess); // Ready Queue�� �ְ�
					deleteProcess(waiting_queue, tempProcess->pid); // Waiting Queue������ ����.
				}
			}
		}

		if (runningProcess != NULL && runningProcess->CPU_burst_time == 0) { // Process�� ���� �Ϸ� �Ǿ� CPU Burst Time�� 0�϶� Terminate�����ش�.
			printf("Time : %d, Process %d Terminated!\n", time, runningProcess->pid);
			processNode* temp_queue = queue;
			int process_burst_time; // �ݺ����� ���鼭 CPU Burst Time�κ��� �ǵ�ȱ� ������ Queue���� �������ش�.
			while (TRUE) {
				temp_queue = temp_queue->next;
				if (temp_queue->process_pointer->pid == runningProcess->pid) {
					process_burst_time = temp_queue->process_pointer->CPU_burst_time; // CPU Burst Time�� �����ͼ� AWT�� ATT ��꿡 ����Ѵ�.
					break;
				}
			}

			if (terminated_process_count == 0) { // AWT ATT ���
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

		if (interrupt_index < INTERRUPT_COUNT && time == IOinterrupt[interrupt_index]) { // I/O Interrupt�� �߻����� �� Process�� Waiting Queue�� �̵���Ų��.
			printf("Time : %d, I/O Interrupted!\n", time);
			interrupt_index++; 
			if (runningProcess != NULL) {
				createProcess(waiting_queue, runningProcess);
				runningProcess = NULL;
			}
		}

		temp_queue = queue; // Scheduling�� �������� �� Process�� Arrived�Ǵ� �ð��� �ľ��� �� �ð��� �Ǹ� Process�� Ready Queue�� �־��ش�.
		while (TRUE) {
			temp_queue = temp_queue->next;
			if (temp_queue == NULL) break;
			if (temp_queue->process_pointer->arrival_time == time) {
				createProcess(ready_queue, temp_queue->process_pointer);
				printf("Time : %d, Process %d has arrived!\n", time, temp_queue->process_pointer->pid);
			}
		}

		if (runningProcess == NULL && ready_queue->next == NULL) { // ���� �������� Process�� ���� Ready Queue�� ������� ���
			if (time >= burst_sum) { // �ð��� ��������
				printf("Scheduling End!"); // Scheduling�� �����Ѵ�.
				break;
			}
			else { // �װ� �ƴϸ� idle�ϰ� ��ٸ���.
				printf("Time : %d, Idle Time\n", time);
				burst_sum++; // idle�� ���� burst_sum�� �÷��ش�.
			}
		}

		if (runningProcess != NULL || ready_queue->next != NULL) { // �� Process�� Run�ϴ� �κ��̴�.
			switch (mode) {
			case FCFS:
				/*
				FCFS�� ���� ���� ���� Process���� ���������� �����Ų��.
				�ڵ忡���� Ready Queue���� ���� �տ� �ִ� Process�� �����´�.
				*/
				if (runningProcess == NULL) {
					runningProcess = ready_queue->next->process_pointer;
					deleteProcess(ready_queue, runningProcess->pid);
				}
				break;
			case NON_PREEMPTIVE_SJF:
				/*
				Non-Preemptive SJF�� ���� ���� Burst Time�� ���� Process���� ���������� �����Ų��.
				�ڵ忡���� Ready Queue�� �ѹ� �Ⱦ�� ���� ���� Burst Time�� ���� Process�� �����´�.
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
				Preemptive SJF�� �⺻������ Non-Preemptive SJF�� ������, ���������̶�� �κп��� ���̰� �ִ�.
				�ڵ忡���� SJF�� ������, ������ �Ͼ ��� runningProcess�� ��ü�ϰ� �۾� ���̴� Process�� Ready Queue�� �ٽ� �ִ´�.
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
				Non-Preemptive Priority�� ���� ���� Priority�� ���� Process���� �����Ѵ�.
				�ڵ忡���� Ready Queue�� �ѹ� �Ⱦ�� ���� ���� Priority�� ���� Process�� �����´�.
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
				Preemptive Priority�� �⺻������ Non-Preemptive Priority�� ������, ���������̶�� �κп��� ���̰� �ִ�.
				�ڵ忡���� Priority�� ������, ������ �Ͼ ��� runningProcess�� ��ü�ϰ� �۾� ���̴� Process�� Ready Queue�� �ٽ� �ִ´�.
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
				Round Robin�� ���� �⺻�����δ� FCFS�� ������ ������, Time Quantum�� �ѵ��� Process�� ����Ǹ� ��ü���ش�.
				�ڵ忡���� Time Quantum�� ���� ������ Ready Queue���� ���� �տ� �ִ� Process�� �������� �۾� ���̴� Process�� Ready Queue�� �ٽ� �ִ´�.
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

		GanttNode* temp = gantt; // time�� 1 ������ ������ GanttChart�� ������Ʈ �����ش�. gantt�� Queue�̴�.
		while (temp->next != NULL) {
			temp = temp->next;
		}

		int temppid = (runningProcess == NULL) ? 0 : runningProcess->pid; // O�̸� Idle�� �����̰�, �׷��� ���� �͵��� �������� process�� id�̴�.

		if (temp == gantt || temp->pid != temppid) { // Queue�� ���κ��� ���� pid�� �ٸ��� ���� �ϳ��� ���� �־��ش�.
			GanttNode* newGantt = (GanttNode*)malloc(sizeof(GanttNode));
			newGantt->time = 1;
			newGantt->pid = temppid;
			newGantt->next = NULL;
			temp->next = newGantt;
		}
		else { // �ƴҰ�� �ð��� 1 ���������ش�.
			temp->time++;
		}

		time++;
	}

	printGanttChart(gantt);

	printf("Average Waiting Time : %.3lf\n", average_waiting_time);
	printf("Average Turnaround Time : %.3lf\n\n\n", average_turnaround_time);

	evaluateMode[mode - 1] = mode; // mode�� ���� ������ ���� �� ������ �ؾ� �ϴµ� AWT�� ATT�� ������ �����ϴٺ��� �� Time�� � Scheduler�� ������ �ľ��ϱ� ����� ����.
	evaluateTime[mode - 1][0] = average_waiting_time;
	evaluateTime[mode - 1][1] = average_turnaround_time;

	return;
}

void evaluateScheduler() {
	for (int i = 0; i < 6; i++) { // Selection �����̴�.
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
	processNode* process_queue = (processNode*)malloc(sizeof(processNode)); // Process�� ���� ������ �ʴ� Queue�� �����.
	process_queue->next = NULL;
	srand(time(NULL));
	for (int i = 0; i < rand() % 5 + 5; i++) { // Random�� ������ Process�� ������ش�. �ڵ忡���� (5 ~ 9)
		initProcess(process_queue);
	}
	int* IOInterrupt;
	IOInterrupt = makeIOInterrupt(process_queue); // I/O Interrupt�� ������ش�.

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