#include <iostream>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <ctime>
#include <Windows.h>
#include <mpi.h>

using namespace std;

int seed;

double RandDouble(int max) {
	srand(time(0) + seed);
	seed = rand();
	return  (double)(rand() % max) + (double)(rand() % max) / max;
}

class Matrix {

public:
	int rowsCount;
	int columnsCount;
	double **elems;

	void GenerateMatrix(int randMax) {
		for (int i = 0; i < rowsCount; i++) {
			for (int j = 0; j < columnsCount; j++)
				elems[i][j] = RandDouble(randMax);
		}
	}

	void Print() {
		for (int i = 0; i < rowsCount; i++) {
			for (int j = 0; j < columnsCount; j++)
				std::cout << " " << elems[i][j];
			std::cout << "\n";
		}
		std::cout << "\n";
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

//Функция, описывающая исполнение подзадачи одним из некорневых процессов 
void ProcTask(int rank) {

	//Буфер для сохранения размеров матрицы B
	int bsizeBuff[2];
	//Буфер для сохранения параметров подзадачи
	int taskParams[4];

	double **matrixARows;
	double **matrixB;
	double *result;
	
	MPI_Status status;

	//Получаем broadcast-сообщение с размерами матрицы B
	MPI_Bcast(&bsizeBuff, 2, MPI_INT, 0, MPI_COMM_WORLD);

	int bRowsCount = bsizeBuff[0];
	int bColumnsCount = bsizeBuff[1];
	matrixB = new double*[bRowsCount];
	//Заполняем матрицу B исходными элементами
	for (int i = 0; i < bRowsCount; i++) {
		matrixB[i] = new double[bColumnsCount];
		//Получаем broadcast-сообщение, содержащее i-ую строку матрицы B
		MPI_Bcast(matrixB[i], bColumnsCount, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	}
	//Получаем broadcast-сообщение с параметрами подзадачи
	MPI_Recv(&taskParams, 4, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);

	//Количество строк матрицы A, необходимые для выполнения текущей подзадачи
	int aRowsCount = taskParams[0]; 
	//Количество столбцов матрицы A
	int aColumnsCount = taskParams[1];
	//Индекс столбца, с которого необходимо начать вычисления
	int startColumnIndex = taskParams[2];
	//Число элементов, которые необходимо вычислить в рамках текущей подзадачи
	int elemsToCalcByProc = taskParams[3];

	matrixARows = new double*[aRowsCount];
	result = new double[elemsToCalcByProc];
	//Получаем все строки матрицы A, необходимые для расчета текущей подзадачи
	for (int i = 0; i < aRowsCount; i++) {
		matrixARows[i] = new double[aColumnsCount];
		MPI_Recv(matrixARows[i], aColumnsCount, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, &status);
	}
	int j = startColumnIndex;
	//Рассчитываем требующееся количество элементов и записываем их в единый буфер
	for (int i = 0, m = 0; i < aRowsCount; i++) {
		for (; j < bColumnsCount && m < elemsToCalcByProc; j++, m++) {
			double tmp = 0;
			for (int k = 0; k < aColumnsCount; k++) {
				tmp += matrixARows[i][k] * matrixB[k][j];
			}
			result[m] = tmp;
		}
		j = 0;
	}
	//Отправляем результаты корневому процессу
	MPI_Send(result, elemsToCalcByProc, MPI_DOUBLE, 0, rank, MPI_COMM_WORLD);

	for (int i = 0; i < bRowsCount; i++)
		delete matrixB[i];
	for (int i = 0; i < aRowsCount; i++)
		delete matrixARows[i];

	delete matrixARows;
	delete matrixB;
	delete result;
}


Matrix* Multiply(Matrix& a, Matrix& b, int procCount) {

	int bsizeBuff[2] = { b.rowsCount, b.columnsCount };

	int *awaitElemsCount = new int[procCount];

	if (a.columnsCount == b.rowsCount) {
		
		Matrix *result = new Matrix(a.rowsCount, b.columnsCount);
		int elemsToCalc = a.rowsCount * b.columnsCount;
		int avCountToCalcByProc = elemsToCalc / procCount;
		int remElemsCount = elemsToCalc % procCount;

		//Рассылаем всем процессам информацию о размере матрицы B
		MPI_Bcast(&bsizeBuff, 2, MPI_INT, 0, MPI_COMM_WORLD);

		//Построчно рассылаем матрицу B всем процессам 
		for (int i = 0; i < b.rowsCount; i++)
			MPI_Bcast(b.elems[i], b.columnsCount, MPI_DOUBLE, 0, MPI_COMM_WORLD);

		int resultRowsCount = result->rowsCount;
		int resultColumnsCount = result->columnsCount;

		for (int tmpCounter = 0, i = 1; tmpCounter < elemsToCalc; i++) {

			int startRowIndex = tmpCounter / resultColumnsCount;
			int startColumnIndex = tmpCounter % resultColumnsCount;
			int elemsToCalcByProc = avCountToCalcByProc;

			if (remElemsCount > 0) {
				elemsToCalcByProc += 1;
				remElemsCount -= 1;
			}

			tmpCounter += elemsToCalcByProc;

			int tmp = tmpCounter / resultColumnsCount - startRowIndex;
			//Определяем число строк матрицы A, необходимых для выполнения формируемой подзадачи
			int rowsCount = (tmpCounter % resultColumnsCount > 0) ? tmp + 1 : tmp;
			awaitElemsCount[i - 1] = elemsToCalcByProc;

			//Формируем и отправляем процессу параметры назначенной ему подзадачи
			int calcParams[4] = { rowsCount, a.columnsCount, startColumnIndex, elemsToCalcByProc };
			MPI_Send(&calcParams, 4, MPI_INT, i, 0, MPI_COMM_WORLD);

			double **aElems = a.elems;
			int aColumnsCount = a.columnsCount;
			//Отправляем процессу необходимые строки матрицы A
			for (int j = startRowIndex, k = 0; k < rowsCount; j++, k++) {
				MPI_Send(aElems[j], aColumnsCount, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
			}

		}

		MPI_Status status;
		int resRowPos = 0, resColumnPos = 0;
		double *resBuff = new double[awaitElemsCount[0]];
		double **resultElems = result->elems;

		//Получаем от каждого процесса, не являющегося корневым, результаты расчетов и формируем результирующую матрицу
		for (int otherProc = 1; otherProc <= procCount; otherProc++)
		{
			int resSize = awaitElemsCount[otherProc - 1];
			MPI_Recv(resBuff, resSize, MPI_DOUBLE, otherProc, otherProc, MPI_COMM_WORLD, &status);
			for (int k = 0; resRowPos < resultRowsCount; ++resRowPos) {
				for (; resColumnPos < resultColumnsCount && k < resSize; ++resColumnPos, ++k) {
					resultElems[resRowPos][resColumnPos] = resBuff[k];
				}
				if (k >= resSize)
					break;
				else 
					resColumnPos = 0;
			}
		}

		delete awaitElemsCount;
		delete resBuff;

		return result;
	}
	else
		return NULL;
}


int main(int argc, char *argv[]) {

	int rank, procCount;

	MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	MPI_Comm_size(MPI_COMM_WORLD, &procCount);

	if (rank == 0) {
		//Выполняется в корневом процессе
		double *buff;
		Matrix a(1500, 500);
		Matrix b(500, 1000);
		a.GenerateMatrix(100);
		b.GenerateMatrix(100);

		unsigned int startTime = clock();

		Matrix *c = Multiply(a, b, procCount - 1);

		unsigned int endTime = clock();
		unsigned int multiplicationTime = endTime - startTime;
		std::cout << multiplicationTime << endl;

		delete c;
	}
	else
		ProcTask(rank); //Выполняется в процессах, обеспечивающих решение подзадач

	MPI_Finalize();
	return 0;
}