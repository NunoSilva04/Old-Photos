Project that incorporates parallel work by the use of threads
To run the program it should be:
./old-photo-parallel-B [directoryName] [number of threads] -[size/name]
directoryName - directory where the pictures are
number of threads - how many threads should be created
size/name - in the output file should they be organized by name or by size

Some examples:
./old-photo-parallel-B "./DataSet 1" 4 -size
./old-photo-parallel-B ./dir-2 8 -name
./old-photo-parallel-B . 1 -name
