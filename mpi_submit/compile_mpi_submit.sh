#   File name: mpi_sub
#   Authors: Roger Tait (roger.tait@bcu.ac.uk)
#   Date created: 10/7/2018

#   Part of the mpi_sub-1.0.0 package 
#   The Software is distributed 'as is' and is solely for non-commercial use

#   Developed at Faculty of Computing, Engineering and the Built Environment
#   Birmingham City Univerity
#   University House
#   Birmingham



module load gcc-10.2.0
module load openmpi-4.1.0



mpicxx mpi_submit.cpp -o mpi_submit
