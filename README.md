#tplog

highly extensible c++ log library on windows system

tplog include: 

 - tplog library; 
 - tplogview, a GUI tool for viewing tplog pipe device output;
 - testtplog, for tplog functional testing.


#dir structure

 * [tplog]          files for wrapping log functionality into DLL
 * [tplogview]      tplogview source
 * [build]          vcprojs for building
 * [doc]            documents
 * [include]        [detail]          tplog implementation
 *                  tplog.h           tplog interface
 *                  tplog_util.h      tplog util interface
 *                  accutime.h        tplog sync time reader
 *                  tplogprovider.h   tplog pipe receiver and file reader (tplogprovider.h)
 * [testtplog]      functional test
 * [thirdsrc]       third party source, only used by tplogview and testtplog
 * [tools]          misc util
