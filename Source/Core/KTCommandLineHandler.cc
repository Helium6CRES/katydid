/*
 * KTCommandLineHandler.cxx
 *
 *  Created on: Nov 21, 2011
 *      Author: nsoblath
 */

#include "KTCommandLineHandler.hh"

#include "KTCommandLineOption.hh"

#include "KatydidConfig.hh"

#include <sstream>

#ifndef PACKAGE_STRING
#define PACKAGE_STRING Katydid (unknown version)
#endif
#define STRINGIFY_1(x) #x
#define STRINGIFY_2(x) STRINGIFY_1(x)

using std::string;

namespace Katydid
{
    KTLOGGER(utillog, "katydid.utility");

    CommandLineHandlerException::CommandLineHandlerException (std::string const& why)
      : std::logic_error(why)
    {}

    KTCommandLineHandler::KTCommandLineHandler() :
            fExecutableName("NONE"),
            fPackageString(STRINGIFY_2(PACKAGE_STRING)),
            fNArgs(0),
            fArgV(NULL),
            fArgumentsTaken(false),
            fCommandLineOptions(),
            fPrintHelpOptions(),
            fCommandLineParseLater(),
            fParsedOptions(NULL),
            fCommandLineVarMap(),
            fPrintHelpMessageAfterConfig(false),
            fConfigFilename()
    {
    }

    KTCommandLineHandler::~KTCommandLineHandler()
    {
        while (! fProposedGroups.empty())
        {
            OptDescMapIt tIter = fProposedGroups.begin();
            delete tIter->second;
            fProposedGroups.erase(tIter);
        }
    }

    Bool_t KTCommandLineHandler::TakeArguments(Int_t argC, Char_t**argV)
    {
        if (fArgumentsTaken) return false;

        fNArgs = argC;
        fArgV = argV;
        fArgumentsTaken = true;

        InitialCommandLineProcessing();

        return true;
    }

    //**************

    Bool_t KTCommandLineHandler::GetArgumentsTaken()
    {
        return fArgumentsTaken;
    }

    Int_t KTCommandLineHandler::GetNArgs()
    {
        return fNArgs;
    }

    Char_t** KTCommandLineHandler::GetArgV()
    {
        return fArgV;
    }

    //**************

    KTCommandLineHandler::OptDescMapIt KTCommandLineHandler::CreateNewOptionGroup(const string& aTitle)
    {
        po::options_description* tNewOpts = new po::options_description(aTitle);
        std::pair< OptDescMapIt, Bool_t > result = fProposedGroups.insert(OptDescMap::value_type(aTitle, tNewOpts));
        if (! result.second)
        {
            KTWARN(utillog, "There is already an option group with title <" << aTitle << ">");
            delete tNewOpts;
        }

        return result.first;
    }

    Bool_t KTCommandLineHandler::AddOption(const string& aTitle, const string& aHelpMsg, const string& aLongOpt, Char_t aShortOpt, Bool_t aWarnOnDuplicate)
    {
        if (fAllOptionsLong.find(aLongOpt) != fAllOptionsLong.end())
        {
            if (aWarnOnDuplicate)
                KTWARN(utillog, "There is already an option called <" << aLongOpt << ">");
            return false;
        }
        if (aShortOpt != '#')
        {
            if (fAllOptionsShort.find(aShortOpt) != fAllOptionsShort.end())
            {
                if (aWarnOnDuplicate)
                        KTWARN(utillog, "There is already a short option called <" << aShortOpt << ">");
                return false;
            }
        }

        // option is okay at this point

        OptDescMapIt tIter = fProposedGroups.find(aTitle);
        if (tIter == fProposedGroups.end())
        {
            tIter = CreateNewOptionGroup(aTitle);
        }

        string tOptionName = aLongOpt;
        fAllOptionsLong.insert(aLongOpt);
        if (aShortOpt != '#')
        {
            tOptionName += "," + string(&aShortOpt);
            fAllOptionsShort.insert(aShortOpt);
        }
        tIter->second->add_options()(tOptionName.c_str(), aHelpMsg.c_str());

        return true;
    }

    Bool_t KTCommandLineHandler::AddOption(const string& aTitle, const string& aHelpMsg, const string& aLongOpt, Bool_t aWarnOnDuplicate)
    {
        if (fAllOptionsLong.find(aLongOpt) != fAllOptionsLong.end())
        {
            if (aWarnOnDuplicate)
                KTWARN(utillog, "There is already an option called <" << aLongOpt << ">");
            return false;
        }

        // option is okay at this point

        OptDescMapIt tIter = fProposedGroups.find(aTitle);
        if (tIter == fProposedGroups.end())
        {
            tIter = CreateNewOptionGroup(aTitle);
        }

        fAllOptionsLong.insert(aLongOpt);
        tIter->second->add_options()(aLongOpt.c_str(), aHelpMsg.c_str());

        return true;
    }

    po::options_description* KTCommandLineHandler::GetOptionsDescription(const string& aKey)
    {
        OptDescMapIt tIter = fProposedGroups.find(aKey);
        if (tIter == fProposedGroups.end())
        {
            KTWARN(utillog, "There no proposed option group with key <" << aKey << ">");
            return NULL;
        }
        return tIter->second;
    }

    //**************

   Bool_t KTCommandLineHandler::FinalizeNewOptionGroups()
    {
        for (OptDescMapIt tIter = fProposedGroups.begin(); tIter != fProposedGroups.end(); tIter++)
        {
            if (! AddCommandLineOptions(*(tIter->second)))
            {
                return false;
            }
            delete tIter->second;
            fProposedGroups.erase(tIter);
        }

        return true;
    }

    Bool_t KTCommandLineHandler::AddCommandLineOptions(const po::options_description& aSetOfOpts)
    {
        try
        {
            fCommandLineOptions.add(aSetOfOpts);
            fPrintHelpOptions.add(aSetOfOpts);
        }
        catch (std::exception& e)
        {
            KTERROR(utillog, "Exception thrown while adding options: " << e.what());
            return false;
        }
        catch (...)
        {
            KTERROR(utillog, "Exception was thrown, but caught in a generic way!");
            return false;
        }
        return true;
    }

    //**************

    Bool_t KTCommandLineHandler::IsCommandLineOptSet(const string& aCLOption)
    {
        return fCommandLineVarMap.count(aCLOption);
    }

    //**************

    void KTCommandLineHandler::DelayedCommandLineProcessing()
    {
        if (! fProposedGroups.empty())
        {
            if (! this->FinalizeNewOptionGroups())
            {
                KTERROR(utillog, "An error occurred while adding the proposed option groups\n" <<
                        "Command-line options were not parsed");
                return;
            }
        }

        // Parse the command line options that remain after the initial parsing
        try
        {
            fParsedOptions = po::command_line_parser(fCommandLineParseLater).options(fCommandLineOptions).run();
        }
        catch (std::exception& e)
        {
            KTERROR(utillog, "An error occurred while boost was parsing the command line options:\n" << e.what());
            return;
        }
        // Create the variable map from the parse options
        po::store(fParsedOptions, fCommandLineVarMap);
        po::notify(fCommandLineVarMap);

        return;
    }

    void KTCommandLineHandler::InitialCommandLineProcessing()
    {
        if (fNArgs <= 0 || fArgV == NULL)
        {
            fNArgs = 0;
            fArgV = NULL;
            return;
        }

        // Get the executable name
        if (fNArgs >= 1) fExecutableName = string(fArgV[0]);

        // If no arguments were given, print the help message and exit
        if (fNArgs == 1) PrintHelpMessageAndExit();

        // Define general options, and add them to the complete option list
        po::options_description tGeneralOpts("General options");
        tGeneralOpts.add_options()("help,h", "Print help message")("help-config", "Print help message after reading config file")("version,v", "Print version information");
        /* WHEN NOT USING POSITIONAL CONFIG FILE ARGUMENT */
        tGeneralOpts.add_options()("config-file,c", po::value< string >(), "Configuration file");
        /**/
        // We want to have the general options printed if --help is used
        fPrintHelpOptions.add(tGeneralOpts);

        // Fill in the duplication-checking sets
        fAllGroupKeys.insert("General");
        fAllOptionsLong.insert("help");
        fAllOptionsShort.insert('h');
        fAllOptionsLong.insert("help-config");
        fAllOptionsLong.insert("version");
        fAllOptionsShort.insert('v');

        // Define the option for the user configuration file; this does not get printed in list of options when --help is used
        po::options_description tHiddenOpts("Hidden options");
        /* WHEN USING POSITIONAL CONFIG FILE ARGUMENT
        tHiddenOpts.add_options()("config-file", po::value< string >(), "Configuration file");
        */
        // Add together any options that will be parsed here, in the initial command-line processing
        po::options_description tInitialOptions("Initial options");
        tInitialOptions.add(tGeneralOpts).add(tHiddenOpts);

        // Allow the UserConfiguration file to be specified with the only positional option
        /* WHEN USING POSITIONAL CONFIG FILE ARGUMENT
        po::positional_options_description tPositionOpt;
        tPositionOpt.add("config-file", 1);
        */

        // Add contributions of other options from elsewhere in Katydid
        FinalizeNewOptionGroups();

        // Command line style
        po::command_line_style::style_t pstyle = po::command_line_style::unix_style;
        /*
        po::command_line_style::style_t pstyle = po::command_line_style::style_t(
                po::command_line_style::allow_long |
                po::command_line_style::allow_short |
                po::command_line_style::allow_dash_for_short |
                po::command_line_style::long_allow_next |
                po::command_line_style::long_allow_adjacent |
                po::command_line_style::short_allow_next |
                po::command_line_style::short_allow_adjacent |
                po::command_line_style::allow_guessing |
                po::command_line_style::allow_sticky);
        */

        po::parsed_options tParsedOpts(NULL);
        // Parse the command line looking only for the general options
        try
        {
            tParsedOpts = po::command_line_parser(fNArgs, fArgV).style(pstyle).options(tInitialOptions).allow_unregistered().run();
            /* WHEN USING POSITIONAL CONFIG FILE ARGUMENT
            tParsedOpts = po::command_line_parser(fNArgs, fArgV).style(pstyle).options(tInitialOptions).positional(tPositionOpt).allow_unregistered().run();
            */
        }
        catch (std::exception& e)
        {
            KTERROR(utillog, "Exception caught while performing initial CL parsing:\n"
                    << '\t' << e.what());
            throw std::logic_error(e.what());
        }
        // Save the remaining command-line options for later parsing (after the full option list has been populated)
        fCommandLineParseLater = po::collect_unrecognized(tParsedOpts.options, po::include_positional);
        /* some debugging couts
        std::cout << "there are " << fCommandLineParseLater.size() << " tokens to parse later." << std::endl;
        for (UInt_t i = 0; i < fCommandLineParseLater.size(); i++)
        {
            std::cout << "   " << fCommandLineParseLater[i] << std::endl;
        }
        */

        // Create the variable map from the general options
        po::variables_map tGeneralOptsVarMap;
        po::store(tParsedOpts, tGeneralOptsVarMap);
        po::notify(tGeneralOptsVarMap);

        // Use the general options information
        if (tGeneralOptsVarMap.count("help"))
        {
            PrintHelpMessageAndExit();
        }
        if (tGeneralOptsVarMap.count("help-config"))
        {
            fPrintHelpMessageAfterConfig = true;
        }
        if (tGeneralOptsVarMap.count("version"))
        {
            PrintVersionMessageAndExit();
        }
        if (tGeneralOptsVarMap.count("config-file"))
        {
            fConfigFilename = tGeneralOptsVarMap["config-file"].as< string >();
        }

        return;
    }

    //**************

    void KTCommandLineHandler::PrintHelpMessageAndExit()
    {
        KTINFO(utillog, "\nUsage: " << fExecutableName << " [options]\n\n" <<
               "  If using a config file, it should be specified as:  -c config_file.json\n" <<
               "  Config file options can be modified using:  --address.of.option=\"value\"\n" <<
               fPrintHelpOptions);

        exit(0);
    }

    void KTCommandLineHandler::PrintVersionMessageAndExit()
    {
        std::stringstream printStream;
        printStream << fExecutableName << " -- Version INformation\n";
        printStream << "Built with: " << fPackageString;
        KTINFO(utillog, printStream.str());
        exit(0);
    }



} /* namespace Katydid */