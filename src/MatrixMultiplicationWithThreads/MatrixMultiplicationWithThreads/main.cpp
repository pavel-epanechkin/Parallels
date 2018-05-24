#include <iostream>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <ctime>
#include <Windows.h>



using namespace std;

int seed;

//��������������� ������� ��� ��������� ������-���������� ����� �������� ��������
double RandDouble(int max) {
	srand(time(0) + seed);
	seed = rand();
	return  (double)(rand() % max) + (double)(rand() % max) / max;
}

//�����, ����������� �������
class Matrix {
public:
	int rowsCount;
	int columnsCount;
	double **elems;

	//��������������� ����� ��� ���������� ������� ���������� �������
	void GenerateMatrix(int randMax) {
		for (int i = 0; i < rowsCount; i++) {
			for (int j = 0; j < columnsCount; j++)
				elems[i][j] = RandDouble(randMax);
		}
	}

	//��������������� ����� ��� ������ ����������� ������� � �������
	void Print() {
		for (int i = 0; i < rowsCount; i++) {
			for (int j = 0; j < columnsCount; j++)
				cout << " " << elems[i][j];
			cout << "\n";
		}
		cout << "\n";
	}

	Matrix(int _rowsCount, int _columnsCount) {
		rowsCount = _rowsCount;
		columnsCount = _columnsCount;
		elems = new double*[rowsCount];
		for (int i = 0; i < rowsCount; i++)
			elems[i] = new double[columnsCount];
	}

	~Matrix() {
		for (int i = 0; i < rowsCount; i++)
			delete elems[i];
		delete elems;
	}
};

//����������� �������
HANDLE *hThreads;
//���������� �������
int threadsCount;

//����� ��� �������� ���������� ��������� ��� ������
class TaskParams {
public:
	//�������� ������� A
	Matrix *a;
	//�������� ������� B
	Matrix *b;
	//�������������� �������
	Matrix *result;
	//�������, ������������ ������� �������� � ������� ����������, � �������� ���������� �������� ����������
	int startRowIndex;
	int startColumnIndex;
	//����� ���������, ������� ������ ���� ���������� � ������ ������� ���������
	int elemsToCalculateCount;

	TaskParams(Matrix *_a, Matrix *_b, Matrix *_result, int _startRowIndex, int _startColumnIndex, int _elemsToCalculateCount) {
		a = _a;
		b = _b;
		result = _result;
		startRowIndex = _startRowIndex;
		startColumnIndex = _startColumnIndex;
		elemsToCalculateCount = _elemsToCalculateCount;
	}
};

//�������, ����������� �������� ��� ������� ������
DWORD WINAPI ThreadTask(CONST LPVOID lpParam) {
	TaskParams task = *((TaskParams*)lpParam);
	int resRowsCount = task.result->rowsCount;
	int resColumnsCount = task.result->columnsCount;
	int aColumnsCount = task.a->columnsCount;
	double **aElems = task.a->elems;
	double **bElems = task.b->elems;
	double **resElems = task.result->elems;
	int j = task.startColumnIndex;
	for (int i = task.startRowIndex; i < resRowsCount; i++) {
		for (; j < resColumnsCount && task.elemsToCalculateCount > 0; j++) {
			double tmp = 0;
			for (int k = 0; k < aColumnsCount; k++)
				tmp += aElems[i][k] * bElems[k][j];
			resElems[i][j] = tmp;
			--task.elemsToCalculateCount;
		}
		j = 0;
	}
	delete lpParam;
	ExitThread(0);
}

//������������ ��������� ������
Matrix* Multiply(Matrix& a, Matrix& b) {
	if (a.columnsCount == b.rowsCount) {
		Matrix *result = new Matrix(a.rowsCount, b.columnsCount);
		//������������ ����� ����� ��������� �������������� �������
		int elemsToCalc = a.rowsCount * b.columnsCount;
		//���������� ���������� ��������� ������� (����� �����), ������� ���������� ���������� ������� �� �������
		int avCountToCalcByThread = elemsToCalc / threadsCount;
		//���������� ������� ��� ���������� ������ ����� ��������� �� �������
		int remElemsCount = elemsToCalc % threadsCount;

		for (int tmpCounter = 0, i = 0; tmpCounter < elemsToCalc; i++) {

			//������������ ������� �������� �������������� �������, � �������� ����� ����� ������ ������ ����������
			int startRowIndex = tmpCounter / result->columnsCount;
			int startColumnIndex = tmpCounter % result->columnsCount;
			int elemsToCalcByThread = avCountToCalcByThread;

			//������������ ���������� �������� �� ������� ��� �� �������
			if (remElemsCount > 0) {
				elemsToCalcByThread += 1;
				remElemsCount -= 1;
			}

			tmpCounter += elemsToCalcByThread;

			//������� � ��������� ����� ��� ������� ���������
			hThreads[i] = CreateThread(
				NULL,
				0,
				(LPTHREAD_START_ROUTINE)(&ThreadTask),
				new TaskParams(&a, &b, result, startRowIndex, startColumnIndex, elemsToCalcByThread),
				0,
				NULL
			);
		}
		//������� ���������� ���� ���������� �������
		WaitForMultipleObjects(threadsCount, hThreads, TRUE, INFINITE);

		for (int i = 0; i < threadsCount; i++) {
			CloseHandle(hThreads[i]);
		}

		return result;
	}
	else
		return NULL;
}


int main(int argc, char *argv[]) {
	threadsCount = 2;

	//��������� ��������, ����������� ���������� �������
	if (argc > 1) {
		threadsCount = atoi(argv[1]);
	}

	Matrix a(1500, 500);
	Matrix b(500, 1000);
	a.GenerateMatrix(100);
	b.GenerateMatrix(100);
	hThreads = new HANDLE[threadsCount];

	unsigned int startTime = clock();

	Matrix *c = Multiply(a, b);

	unsigned int endTime = clock();
	unsigned int multiplicationTime = endTime - startTime;
	cout << multiplicationTime << endl;

	return 0;
}