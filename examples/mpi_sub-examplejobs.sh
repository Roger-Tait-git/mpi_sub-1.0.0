#!/bin/sh


#   File name: mpi_sub-examplejobs.sh
#   Authors: Roger Tait (roger.tait@bcu.ac.uk)
#   Date created: 15/3/2023

#   Part of the mpi_sub-1.0.0 training material
#   The Software is distributed 'as is' and is solely for non-commercial use

#   Developed at Faculty of Computing, Engineering and the Built Environment
#   Birmingham City Univerity
#   University House
#   Birmingham



# Define and export in .bashrc MPISUBDIR before starting
MPISUBDIR='/home/admin/mpi_sub-1.0.0'



###########################################################################
#
# Create a dummy task file of 1 job and submit it to slurm!
#
###########################################################################
/bin/rm -f batch1
echo "stress --cpu 1 --timeout 120s" >> batch1
chmod 0700 batch1
batch1_id=`$MPISUBDIR/bin/mpi_sub -T 2 -N batch1 -t ./batch1`
echo Running batch1: ID=$batch1_id



###########################################################################
#
# Create a dummy task file of 100 jobs (spanning servers) and submit it to slurm!
# Wait till batch1 has completed
#
###########################################################################
/bin/rm -f batch100
for i in `seq 1 100`; do
    echo "stress --cpu 1 --timeout 120s" >> batch100
done
chmod 0700 batch100
batch100_id=`$MPISUBDIR/bin/mpi_sub -j $batch1_id -T 2 -N batch100 -t ./batch100`
echo Running batch100: ID=$batch100_id


###########################################################################
#
# Create a self submitting script called scriptnotbatch, that generated a dummy task file of 10 jobs and submits it to slurm!
# Wait till batch100 has completed
#
###########################################################################
cat << start_of_script > scriptnotbatch
#!/bin/sh

/bin/rm -f scriptnotbatch
for i in \`seq 1 10\`; do
    echo "stress --cpu 1 --timeout 120s" >> batchfromscript
done
chmod 0700 batchfromscript
batchfromscript_id=\`$MPISUBDIR/bin/mpi_sub -T 2 -N batchfromscript -t ./batchfromscript\`
echo Running batchfromscript: ID=\$batchfromscript_id

start_of_script

chmod 0700 scriptnotbatch
scriptnotbatch_id=`$MPISUBDIR/bin/mpi_sub -j $batch100_id -T 2 -N scriptnotbatch -t ./scriptnotbatch`
echo Running scriptnotbatch: ID=$scriptnotbatch_id


