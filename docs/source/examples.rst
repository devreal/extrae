.. _cha:Examples:

Examples
========

Here we present three different examples of generating a |PARAVER| tracefile.
The first example requires the package to be compiled with DynInst libraries.
The second example uses the ``LD_PRELOAD`` or ``LDR_PRELOAD[64]`` mechanism to
interpose code in the application. Such mechanism is available in Linux and
FreeBSD operating systems and only works when the application uses dynamic
libraries.  Finally, there is an example using the static library of the
instrumentation package.


.. _sec:Examples_DynInst:

DynInst based examples
----------------------

DynInst is a third-party instrumentation library developed at UW Madison which
can instrument in-memory binaries. It adds flexibility to add instrumentation to
the application without modifying the source code. DynInst is ported to
different systems (Linux, FreeBSD) and to different architectures [#DYNINST]_
(x86, x86/64, PPC32, PPC64) but the functionality is common to all of them.


.. _subsec:Examples_DynInst_Intermediate:

Generating intermediate files for serial or OpenMP applications
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: sh
  :linenos:
  :caption: run_dyninst.sh

  #!/bin/sh

  export EXTRAE_HOME=WRITE-HERE-THE-PACKAGE-LOCATION
  export LD_LIBRARY_PATH=${EXTRAE_HOME}/lib
  source ${EXTRAE_HOME}/etc/extrae.sh

  ## Run the desired program
  ${EXTRAE_HOME}/bin/extrae -config extrae.xml $*

A similar script can be found in ``${EXTRAE_HOME}/share/example/SEQ`` just tune
the :envvar:`EXTRAE_HOME` environment variable and make the script executable
(using :command:`chmod u+x`). You can either pass the XML configuration file
through the :envvar:`EXTRAE_CONFIG_FILE` instead, if you prefer. Line no. 5 is
responsible for loading all the environment variables needed for the DynInst
launcher (called :command:`extrae`) that is invoked in line 8.

In fact, there are two examples provided in
``${EXTRAE_HOME}/share/example/SEQ``, one for static (or manual) instrumentation
and another for the DynInst-based instrumentation.  When using the DynInst
instrumentation, the user may add new routines to instrument using the existing
:ref:`function-list <sec:XMLSectionUF>` file that is already pointed by the
:ref:`extrae.xml <cha:xml>` configuration file. The way to specify the routines
to instrument is add as many lines with the name of every routine to be
instrumented.

Running OpenMP applications using DynInst is rather similar to serial codes.
Just compile the application with the appropriate OpenMP flags and run as
before.  You can find an example in ``${EXTRAE_HOME}/share/example/OMP``.

.. _subsec:Examples_DynInst_Intermediate_MPI:

Generating intermediate files for MPI applications
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

MPI applications can also be instrumented using the DynInst instrumentator. The
instrumentation is done independently to each spawned MPI process, so in order
to execute the DynInst-based instrumentation package on a MPI application, you
must be sure that your MPI launcher supports running shell-scripts. The
following scripts show how to run the DynInst instrumentator from the MOAB/Slurm
queue system. The first script just sets the environment for the job whereas the
second is responsible for instrumenting every spawned task.

.. code-block:: sh
  :linenos:
  :caption: slurm_trace.sh

  #!/bin/bash

  # @ initialdir = .
  # @ output = trace.out
  # @ error =  trace.err
  # @ total_tasks = 4
  # @ cpus_per_task = 1
  # @ tasks_per_node = 4
  # @ wall_clock_limit = 00:10:00
  # @ tracing = 1

  srun ./run.sh ./mpi_ping

The most important thing in the previous script is the line number 11, which is
responsible for spawning the MPI tasks (using the srun command). The spawn
method is told to execute :command:`./run.sh ./mpi_ping` which in fact refers to
instrument the ``mpi_ping`` binary using the ``run.sh`` script. You must adapt
this file to your queue-system (if any) and to your MPI submission mechanism
(i.e., change srun to mpirun, mpiexec, poe, *etc.*).  Note that changing the
line 11 to read like :command:`./run.sh srun ./mpi_ping` would result in
instrumenting the ``srun`` application not ``mpi_ping``.

.. code-block:: sh
  :linenos:
  :caption: run.sh

  #!/bin/bash

  export EXTRAE_HOME=@sub_PREFIXDIR@
  source ${EXTRAE_HOME}/etc/extrae.sh

  # Only show output for task 0, others task send output to /dev/null
  if test "${SLURM_PROCID}" == "0" ; then
    ${EXTRAE_HOME}/bin/extrae -config ../extrae.xml $@ > job.out 2> job.err
  else
    ${EXTRAE_HOME}/bin/extrae -config ../extrae.xml $@ > /dev/null 2> /dev/null
  fi

This is the script responsible for instrumenting a single MPI task. In line
number 4 we set-up the instrumentation environment by executing the commands
from ``extrae.sh``. Then we execute the binary passed to the ``run.sh`` script
in lines 8 and 10. Both lines are executing the same command except that line 8
sends all the output to two different files (one for standard output and another
for standard error) and line 10 sends all the output to ``/dev/null``.

Please note, this script is particularly adapted to the MOAB/Slurm queue
systems. You may need to adapt the script to other systems by using the
appropiate environment variables. Particularly, ``SLURM_PROCID`` identifies the
MPI task id (i.e., the task rank) and may be changed to the proper environemnt
variable (``MPI_RANK`` in ParaStation/Torque/MOAB system or MXMPI_ID in systems
having Myrinet MX devices, for example).


.. _sec:Examples_LDPRELOAD:

LD_PRELOAD based examples
-------------------------

:envvar:`LD_PRELOAD` (or :envvar:`LDR_PRELOAD[64]` in AIX) interposition
mechanism only works for binaries that are linked against shared libraries. This
interposition is done by the runtime loader by substituting the original symbols
by those provided by the instrumentation package. This mechanism is known to
work on Linux, FreeBSD and AIX operating systems, although it may be available
on other operating systems (even using different names [#LD_PRELOAD]_) they are
not tested.

We show how this mechanism works on Linux (or similar environments) in
:ref:`subsec:Examples_Linux` and on AIX in :ref:`subsec:Examples_AIX`.


.. _subsec:Examples_Linux:

Linux
^^^^^

The following script preloads the :file:`libmpitrace` library to instrument MPI
calls of the application passed as an argument (tune :envvar:`EXTRAE_HOME`
according to your installation).

.. code-block:: sh
  :linenos:
  :caption: trace.sh
  :name: trace_preload.sh
  
  #!/bin/sh
  
  export EXTRAE_HOME=<WRITE-HERE-THE-PACKAGE-LOCATION>
  export EXTRAE_CONFIG_FILE=extrae.xml
  export LD_PRELOAD=${EXTRAE_HOME}/lib/libmpitrace.so
  
  ## Run the desired program
  $*

The previous script can be found in
``${EXTRAE_HOME}/share/example/MPI/ld-preload`` in your tracing package
directory. Copy the script to one of your directories, tune the
:envvar:`EXTRAE_HOME` environment variable and make the script executable (using
:command:`chmod u+x`). Also copy the XML configuration ``extrae.xml`` file from
``${EXTRAE_HOME}/share/example/MPI`` to the current directory. This file is used
to configure the whole behavior of the instrumentation package (there is more
information about the XML file on chapter :ref:`cha:XML`). The last line in the
script, :command:`$*`, executes the arguments given to the script, so as you can
run the instrumentation by simply adding the script in between your execution
command.

Regarding the execution, if you run MPI applications from the command-line, you
can issue the typical :command:`mpirun` command as:

.. code-block:: sh

  ${MPI_HOME}/bin/mpirun -np N ./trace.sh mpi-app

where ``${MPI_HOME}`` is the directory for your MPI installation, ``N`` is the
number of MPI tasks you want to run, and ``mpi-app`` is the binary of the MPI
application you want to run.

However, if you execute your MPI applications through a queue system you may
need to write a submission script. The following script is an example of a
submission script for MOAB/Slurm queuing system using the aforementioned
:ref:`trace_preload.sh` script for an execution of the ``mpi-app`` on two
processors.

.. code-block:: sh
  :linenos:
  :caption: slurm-trace.sh
  
  #! /bin/bash
  
  #@ job_name         = trace_run
  #@ output           = trace_run%j.out
  #@ error            = trace_run%j.out
  #@ initialdir       = .
  #@ class            = bsc_cs
  #@ total_tasks      = 2
  #@ wall_clock_limit = 00:30:00
  
  srun ./trace.sh mpi_app

If your system uses LoadLeveler your job script may look like:

.. code-block:: sh
  :linenos:
  :caption: ll.sh
  
  #! /bin/bash
  #@ job_type = parallel
  #@ output = trace_run.ouput
  #@ error = trace_run.error
  #@ blocking = unlimited
  #@ total_tasks = 2
  #@ class = debug
  #@ wall_clock_limit = 00:10:00
  #@ restart = no
  #@ group = bsc41 
  #@ queue
  
  export MLIST=/tmp/machine_list ${$}
  /opt/ibmll/LoadL/full/bin/ll_get_machine_list > ${MLIST}
  set NP = `cat ${MLIST} | wc -l`
  
  ${MPI_HOME}/mpirun -np ${NP} -machinefile ${MLIST} ./trace.sh ./mpi-app
  
  rm ${MLIST}

Besides the job specification given in lines 1-11, there are commands of
particular interest. Lines 13-15 are used to know which and how many nodes are
involved in the computation. Such information information is given to the
:command:`mpirun` command to proceed with the execution. Once the execution
finished, the temporal file created on line 14 is removed on line 19.


.. _subsec:Examples_CUDA:

CUDA
^^^^

There are two ways to instrument CUDA applications, depending on how the package
was configured. If the package was configured with :option:`--with-cuda` only
interposition on binaries using shared libraries are available. If the package
was configured with :option:`--with-cupti` any kind of binary can be
instrumented because the instrumentation relies on the CUPTI library to
instrument CUDA calls. The example shown below is intended for the former case.

.. code-block:: sh
  :linenos:
  :caption: run.sh
  :name: run_cuda.sh

  #!/bin/bash

  export EXTRAE_HOME=/home/harald/extrae
  export PAPI_HOME=/home/harald/aplic/papi/4.1.4

  EXTRAE_CONFIG_FILE=extrae.xml
  LD_LIBRARY_PATH=${EXTRAE_HOME}/lib:${PAPI_HOME}/lib:${LD_LIBRARY_PATH} ./hello
  ${EXTRAE_HOME}/bin/mpi2prv -f TRACE.mpits -e ./hello

In this example, the hello application is compiled using the :command:`nvcc`
compiler and linked against the :file:`cudatrace` library
(:option:`-lcudatrace`). The binary contains calls to ``Extrae_init`` and
``Extrae_fini`` and then executes a CUDA kernel.  Line number 6 refers to the
execution of the application itself.  The |TRACE| configuration file and the
location of the shared libraries are set in this line. Line number 7 invokes the
merge process to generate the final tracefile.


.. _subsec:Examples_AIX:

AIX
^^^

AIX typically ships with POE and LoadLeveler as MPI implementation and queue
system respectively. An example for a system with these software packages is
given below. Please, note that the example is intended for 64 bit applications,
if using 32 bit applications then :envvar:`LDR_PRELOAD64` needs to be changed in
favour of :envvar:`LDR_PRELOAD`.

.. code-block:: sh
  :linenos:
  :caption: ll-aix64.sh

  #@ job_name = basic_test
  #@ output = basic_stdout
  #@ error = basic_stderr
  #@ shell = /bin/bash
  #@ job_type = parallel
  #@ total_tasks = 8
  #@ wall_clock_limit = 00:15:00
  #@ queue

  export EXTRAE_HOME=<WRITE-HERE-THE-PACKAGE-LOCATION>
  export EXTRAE_CONFIG_FILE=extrae.xml
  export LDR_PRELOAD64=${EXTRAE_HOME}/lib/libmpitrace.so

  ./mpi-app

Lines 1-8 contain a basic LoadLeveler job definition. Line 10 sets the |TRACE|
package directory in :envvar:`EXTRAE_HOME` environment variable. Follows setting
the XML configuration file that will be used to set up the tracing. Then follows
setting :envvar:`LDR_PRELOAD64` which is responsible for instrumentation using
the shared library :file:`libmpitrace.so`. Finally, line 14 executes the
application binary.


.. _sec:Examples_static:

Statically linked based examples
--------------------------------

This is the basic instrumentation method suited for those installations that
neither support DynInst nor :envvar:`LD_PRELOAD`, or require adding some manual
calls to the |TRACE| API.


.. _subsec:Examples_static_link:

Linking the application
^^^^^^^^^^^^^^^^^^^^^^^

To get the instrumentation working on your code, first you have to link your
application with the |TRACE| libraries. There are installed examples in your
package distribution under :file:`${EXTRAE_HOME}/share/examples`. There you can
find MPI, OpenMP, pthread and sequential examples depending on the support at
configure time.

Consider the example Makefile found in
:file:`${EXTRAE_HOME}/share/examples/MPI/static`:

.. code-block:: sh
  :linenos:
  :caption: Makefile

  MPI_HOME = /gpfs/apps/MPICH2/mx/1.0.7..2/64
  EXTRAE_HOME = /home/bsc41/bsc41273/foreign-pkgs/extrae-11oct-mpich2/64
  PAPI_HOME = /gpfs/apps/PAPI/3.6.2-970mp-patched/64
  XML2_LDFLAGS = -L/usr/lib64
  XML2_LIBS = -lxml2

  F77 = $(MPI_HOME)/bin/mpif77 
  FFLAGS = -O2
  FLIBS = $(EXTRAE_HOME)/lib/libmpitracef.a \
          -L$(PAPI_HOME)/lib -lpapi -lperfctr \
          $(XML2_LDFLAGS) $(XML2_LIBS)

  all: mpi_ping

  mpi_ping: mpi_ping.f
            $(F77) $(FFLAGS) mpi_ping.f $(FLIBS) -o mpi_ping

  clean:
         rm -f mpi_ping *.o pingtmp? TRACE.*

Lines 2-5 are definitions of some Makefile variables to set up the location of
different packages needed by the instrumentation. In particular,
:envvar:`EXTRAE_HOME` sets where the |TRACE| package directory is located. In
order to link your application with |TRACE| you have to add its libraries in the
link stage (see lines 9-11 and 16). Besides file:`libmpitracef.a` we also add
some PAPI libraries (:option:`-lpapi`), and its dependency (which you may or not
need (:option:`-lperfctr`), the libxml2 parsing library (:option:`-lxml2`), and
finally, the bfd and liberty libraries (:option:`-lbfd` and :option:`-liberty`),
if the instrumentation package was compiled to support merge after trace (see
chapter :ref:`cha:Configuration` for further information).


.. _subsec:Examples_static_Intermediate:

Generating the intermediate files
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Executing an application with the statically linked version of the
instrumentation package is very similar as the method shown in
:ref:`sec:Examples_LDPRELOAD`. There is, however, a difference: do not set
:envvar:`LD_PRELOAD` in :ref:`trace_static.sh`.

.. code-block:: sh
  :linenos:
  :caption: trace.sh
  :name: trace_static.sh

  #!/bin/sh

  export EXTRAE_HOME=WRITE-HERE-THE-PACKAGE-LOCATION
  export EXTRAE_CONFIG_FILE=extrae.xml
  export LD_LIBRARY_PATH=${EXTRAE_HOME}/lib: \
                         /gpfs/apps/MPICH2/mx/1.0.7..2/64/lib: \
                         /gpfs/apps/PAPI/3.6.2-970mp-patched/64/lib

  ## Run the desired program
  $*

See section :ref:`sec:Examples_LDPRELOAD` to know how to run this script either
through command line or queue systems.


.. _sec:Examples_LDPRELOAD_Final:

Generating the final tracefile
------------------------------

Independently from the tracing method chosen, it is necessary to translate the
intermediate tracefiles into a |PARAVER| tracefile. The |PARAVER| tracefile can
be generated automatically (if the tracing package and the XML configuration
file were set up accordingly, see chapters :ref:`cha:Configuration` and
:ref:`cha:XML`) or manually. In case of using the automatic merging process, it
will use all the resources allocated for the application to perform the merge
once the application ends.

To manually generate the final |PARAVER| tracefile issue the following command:

.. code-block:: sh

  ${EXTRAE_HOME}/bin/mpi2prv -f TRACE.mpits -e mpi-app -o trace.prv

This command will convert the intermediate files generated in the previous step
into a single |PARAVER| tracefile. The :file:`TRACE.mpits` is a file generated
automatically by the instrumentation and contains a reference to all the
intermediate files generated during the execution run. The :option:`-e`
parameter receives the application binary ``mpi-app`` in order to perform
translations from addresses to source code. To use this feature, the binary must
have been compiled with debugging information. Finally, the :option:`-o` flag
tells the merger how the |PARAVER| tracefile will be named (:file:`trace.prv` in
this case).


.. rubric:: Footnotes

.. [#DYNINST] The IA-64 architecture support was dropped in DynInst 7.0.

.. [#LD_PRELOAD] Look at
  http://www.fortran-2000.com/ArnaudRecipes/sharedlib.html for further
  information.
