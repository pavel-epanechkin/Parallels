#include <iostream>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <ctime>

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

//�������� ����������������� ��������� ���� ������
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
	//��������� ��� ������� ��������������� ��������
	Matrix a(1500, 500);
	Matrix b(500, 1000);
	//��������� ������� ���������� �������� ������� �� 0 �� 100
	a.GenerateMatrix(100);
	b.GenerateMatrix(100);
	//�������� ��������� ����� ������ ���������� ��������� ������ � �������������
	unsigned int startTime = clock();
	//�������� �������
	Matrix *c = Multiply(a, b);
	//�������� ����� ��������� ���������� ��������� ������
	unsigned int endTime = clock();
	//������������ ����� ����� ���������� ����������������� ��������� ������
	unsigned int multiplicationTime = endTime - startTime;
	cout << multiplicationTime << endl;
	delete c;
	return 0;
}