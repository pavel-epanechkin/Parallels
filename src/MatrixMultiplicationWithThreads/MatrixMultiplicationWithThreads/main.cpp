#include <iostream>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <ctime>
#include <Windows.h>



using namespace std;

int seed;

//Вспомогательная функция для генерации псевдо-случайного числа двоичной точности
double RandDouble(int max) {
	srand(time(0) + seed);
	seed = rand();
	return  (double)(rand() % max) + (double)(rand() % max) / max;
}

//Класс, описывающий матрицу
class Matrix {
public:
	int rowsCount;
	int columnsCount;
	double **elems;

	//Вспомогательный метод для заполнения матрицы случайными числами
	void GenerateMatrix(int randMax) {
		for (int i = 0; i < rowsCount; i++) {
			for (int j = 0; j < columnsCount; j++)
				elems[i][j] = RandDouble(randMax);
		}
	}

	//Вспомогательный метод для вывода содержимого матрицы в консоль
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

//Дескрипторы потоков
HANDLE *hThreads;
//Количество потоков
int threadsCount;

//Класс для описания параметров подзадачи для потока
class TaskParams {
public:
	//Исходная матрица A
	Matrix *a;
	//Исходная матрица B
	Matrix *b;
	//Результирующая матрица
	Matrix *result;
	//Индексы, определяющие позицию элемента в матрице результата, с которого необходимо начинать вычисления
	int startRowIndex;
	int startColumnIndex;
	//Число элементов, которые должны быть рассчитаны в рамках текущей подзадачи
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

//Функция, описывающая позадачу для каждого потока
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

//Параллельное умножение матриц
Matrix* Multiply(Matrix& a, Matrix& b) {
	if (a.columnsCount == b.rowsCount) {
		Matrix *result = new Matrix(a.rowsCount, b.columnsCount);
		//Рассчитываем общее число элементов результирующей матрицы
		int elemsToCalc = a.rowsCount * b.columnsCount;
		//Определяем количество элементов матрицы (целое число), которые необходимо рассчитать каждому из потоков
		int avCountToCalcByThread = elemsToCalc / threadsCount;
		//Определяем остаток при разделении общего числа элементов по потокам
		int remElemsCount = elemsToCalc % threadsCount;

		for (int tmpCounter = 0, i = 0; tmpCounter < elemsToCalc; i++) {

			//Рассчитываем индексы элемента результирующей матрицы, с которого новый поток должен начать вычисление
			int startRowIndex = tmpCounter / result->columnsCount;
			int startColumnIndex = tmpCounter % result->columnsCount;
			int elemsToCalcByThread = avCountToCalcByThread;

			//Распределяем оставшиеся элементы по потокам при их наличии
			if (remElemsCount > 0) {
				elemsToCalcByThread += 1;
				remElemsCount -= 1;
			}

			tmpCounter += elemsToCalcByThread;

			//Создаем и запускаем поток для решения подзадачи
			hThreads[i] = CreateThread(
				NULL,
				0,
				(LPTHREAD_START_ROUTINE)(&ThreadTask),
				new TaskParams(&a, &b, result, startRowIndex, startColumnIndex, elemsToCalcByThread),
				0,
				NULL
			);
		}
		//Ожидаем завершения всех запущенных потоков
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

	//Считываем параметр, описывающий количество потоков
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