/*
 * Katydid.cc
 *
 *  Created on: Sep 28, 2012
 *      Author: nsoblath
 *
 *  This program will run any processor-based code in Katydid.
 *  All of the action is setup with a config file.
 *  See KTProcessorToolbox for details on the configuration option.
 */

#include "KTApplication.hh"
#include "KTLogger.hh"
#include "KTProcessorToolbox.hh"

#include <string>

using namespace std;
using namespace Katydid;

KTLOGGER(katydidlog, "katydid.applications.main");


int main(int argc, char** argv)
{
    KTPROG(katydidlog, "Welcome to Katydid!");
    KTDEBUG(katydidlog,
            "\n" <<
            "                                                                            ,=  \n" <<
            "                                                                           =    \n" <<
            "                                                                          7     \n" <<
            "                                       .:.                               I      \n" <<
            "                                          I?                           :~       \n" <<
            "       +$                                   ~+                        I,        \n" <<
            "        $Z        .,=IZ8NNND$INM,             :7.                   ,I          \n" <<
            "         7OONDO7+~  ,=7$Z88$Z?=,?N               $                 :=           \n" <<
            "             .,?NN?             ~IZMDO$?~         .7              =,            \n" <<
            "          :ZM7,                       ~7, +7Z$$7$Z8NI            $              \n" <<
            "        MN.                              Z=          D?,       .D               \n" <<
            "          +M                                787,     .Z?:   . 7$                \n" <<
            "            ,M$                                 ?Z    :D$   ,Z                  \n" <<
            "               ,M=                                 +7.:7OO?$                    \n" <<
            "                  7O,                                IZ78O: +M:                 \n" <<
            "                     +O8D8~                        .,   M     ?M:               \n" <<
            "                   ,M7     =:MNO,                 :DIDID:        M8M$           \n" <<
            "                .+M:             ,:IIZZZ7==  ,?OMI +O                           \n" <<
            "               7N                         IMO~      ZD.                         \n" <<
            "            =M=                          M,          ~M=                        \n" <<
            "          7D                            M             .IZ                       \n" <<
            "        8$                            .Z:                M                      \n" <<
            "    .ZN=                             .8I                . M=N                   \n" <<
            "MDNNZ                                D=                                         \n" <<
            "                                    $=                                          \n" <<
            "                                    =                                           \n");

    KTApplication app(argc, argv);
    app.ReadConfigFile();

    // Create and configure the processor toolbox.
    // This will create all of the requested processors, connect their signals and slots, and fill the run queue.
    string appConfigName("katydid");
    KTProcessorToolbox procTB;
    if (! app.Configure(&procTB, appConfigName))
    {
        KTERROR(katydidlog, "Unable to configure processor toolbox. Aborting.");
        return -1;
    }

    // Configure the processors
    if (! procTB.ConfigureProcessors(app.GetNode(appConfigName)))
    {
        KTERROR(katydidlog, "Unable to configure processors. Aborting.");
        return -2;
    }

    // Execute the run queue!
    Bool_t success = procTB.Run();

    KTPROG(katydidlog, "That's all, folks!");

    if (! success) return -3;
    return 0;
}