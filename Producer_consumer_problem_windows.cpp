#include <windows.h>
#include <cstdio>

#ifndef BUF_SIZE
#define BUF_SIZE 4
#endif

#ifndef DATA_SIZE
#define DATA_SIZE 3
#endif

#ifndef MAX_SLEEP_TIME
#define MAX_SLEEP_TIME 3000
#endif

//buffer
char buffer[BUF_SIZE];
//data
char data[DATA_SIZE];

//buffer cursor
int i = 0, j = 0;

//semaphores
HANDLE empty, full, mutex;

DWORD WINAPI producer(LPVOID);
DWORD WINAPI consumer(LPVOID);
void insert_item(const char& item, int &i);
void remove_item(char& item, int &j);

int main(void)
{
	//Init
	memset(buffer, '-', sizeof(buffer));
	sprintf(data, "WQY");
	HANDLE producers[3], consumers[4];
	DWORD p_tids[3], c_tids[4];
	empty = CreateSemaphore(NULL, BUF_SIZE, BUF_SIZE, NULL);
	full = CreateSemaphore(NULL, 0, BUF_SIZE, NULL);
	mutex = CreateSemaphore(NULL, 1, 1, NULL);

	//create 3 producers
	for (int i = 0; i < 3; i++)
	{
		producers[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)producer, NULL, 0, &p_tids[i]);
	}

	//create 4 consumers
	for (int i = 0; i < 4; i++)
	{
		consumers[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)consumer, NULL, 0, &c_tids[i]);
	}

	// Wait for all threads to terminate
	WaitForMultipleObjects(3, producers, TRUE, INFINITE);
	WaitForMultipleObjects(4, consumers, TRUE, INFINITE);

	// Close thread and semaphore handles

	for (i = 0; i < 3; ++i)
		CloseHandle(producers[i]);
	for (i = 0; i < 4; ++i)
		CloseHandle(consumers[i]);

	CloseHandle(empty);
	CloseHandle(full);
	CloseHandle(mutex);
	return 0;
}

DWORD WINAPI producer(LPVOID lpParam)
{
	// lpParam not used in this example
	UNREFERENCED_PARAMETER(lpParam);
	int ctr = 0;
	while (TRUE)
	{
		WaitForSingleObject(empty, INFINITE);
		WaitForSingleObject(mutex, INFINITE);

		//wait for a random amount of time
		Sleep(rand() % MAX_SLEEP_TIME);

		insert_item(data[rand() % DATA_SIZE], i);

		ReleaseSemaphore(full, 1, NULL);
		ReleaseSemaphore(mutex, 1, NULL);

		++ctr;
		if (ctr == 4)
		{
			break;
		}
	}
	return TRUE;
}

DWORD WINAPI consumer(LPVOID lpParam)
{
	// lpParam not used in this example
	UNREFERENCED_PARAMETER(lpParam);
	int ctr = 0;
	while (TRUE)
	{
		WaitForSingleObject(full, INFINITE);
		WaitForSingleObject(mutex, INFINITE);

		//wait for a random amount of time
		Sleep(rand() % MAX_SLEEP_TIME);

		//put the actual data in buffer
		char taken;
		remove_item(taken, j);

		ReleaseSemaphore(empty, 1, NULL);
		ReleaseSemaphore(mutex, 1, NULL);

		++ctr;
		if (ctr == 3)
			break;
	}
	return TRUE;
}

void insert_item(const char& item, int &i)
{
	buffer[i] = item;
	//circular buffer
	i = (i + 1) % BUF_SIZE;

	//display info
	printf("Producer %d put item %c from the buffer, current buffer:\n", GetCurrentThreadId(), item);
	for (int k = 0; k < BUF_SIZE; ++k) {
		printf("%c", buffer[k]);
	}
	printf("\n");
}

void remove_item(char& item, int &j)
{
	//get the item
	item = buffer[j];

	//clear the position
	buffer[j] = '-';

	j = (j + 1) % BUF_SIZE;

	//display info
	printf("Consumer %d took item %c from the buffer, current buffer:\n", GetCurrentThreadId(), item);
	for (int k = 0; k < BUF_SIZE; ++k) {
		printf("%c", buffer[k]);
	}
	printf("\n");
}


//https://msdn.microsoft.com/en-us/library/windows/desktop/ms686946(v=vs.85).aspx
//https://msdn.microsoft.com/en-us/library/windows/desktop/ms682516(v=vs.85).aspx