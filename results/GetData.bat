FOR /L %%i IN (1,1,50) DO MatrixMultiplication.exe >> NoThreads.txt

FOR /L %%i IN (1,1,8) DO (
	echo Threads count: %%i >> Threads.txt
	FOR /L %%j IN (1,1,50) DO MatrixMultiplicationWithThreads.exe %%i >> Threads.txt
)

FOR /L %%i IN (2,1,9) DO (
	echo Processes count: %%i >> MPI.txt
	FOR /L %%j IN (1,1,50) DO mpiexec -n %%i MatrixMultiplicationWithMPI.exe >> MPI.txt
)
