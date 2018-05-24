#include <iostream>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <ctime>

using namespace std;

int seed;

//¬спомогательна€ функци€ дл€ генерации псевдо-случайного числа двоичной точности
double RandDouble(int max) {
	srand(time(0) + seed);
	seed = rand();
	return  (double)(rand() % max) + (double)(rand() % max) / max;
}

// ласс, описывающий матрицу
class Matrix {
public:
	int rowsCount;
	int columnsCount;
	double **elems;

	//¬спомогательный метод дл€ заполнени€ матрицы случайными числами
	void GenerateMatrix(int randMax) {
		for (int i = 0; i < rowsCount; i++) {
			for (int j = 0; j < columnsCount; j++)
				elems[i][j] = RandDouble(randMax);
		}
	}

	//¬спомогательный метод дл€ вывода содержимого матрицы в консоль
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

//‘унккци€ последовательного умножени€ двух матриц
Matrix* Multiply(Matrix& a, Matrix& b) {
	if (a.columnsCount == b.rowsCount) {
		Matrix *result = new Matrix(a.rowsCount, b.columnsCount);
		int resRowsCount = result->rowsCount;
		int resColumnsCount = result->columnsCount;
		int aColumnsCount = a.columnsCount;
		double **aElems = a.elems;
		double **bElems = b.elems;
		for (int i = 0; i < resRowsCount; i++) {
			for (int j = 0; j < resColumnsCount; j++) {
				double tmp = 0;
				for (int k = 0; k < aColumnsCount; k++)
					tmp += aElems[i][k] * bElems[k][j];
				result->elems[i][j] = tmp;
			}
		}
		return result;
	}
	else
		return NULL;
}

int main(int argc, char *argv[]) {
	//ќбъ€вл€ем две матрицы соответствующих размеров
	Matrix a(1500, 500);
	Matrix b(500, 1000);
	//«аполн€ем матрицы случайными дробными числами от 0 до 100
	a.GenerateMatrix(100);
	b.GenerateMatrix(100);
	//ѕолучаем системное врем€ начала выполнени€ умножени€ матриц в миллисекундах
	unsigned int startTime = clock();
	//”множаем матрицы
	Matrix *c = Multiply(a, b);
	//ѕолучаем врем€ окончани€ выполнени€ умножени€ матриц
	unsigned int endTime = clock();
	//–ассчитываем общее врем€ выполнени€ последовательного умножени€ матриц
	unsigned int multiplicationTime = endTime - startTime;
	cout << multiplicationTime << endl;
	delete c;
	return 0;
}