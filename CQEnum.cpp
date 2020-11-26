//
//  This is the main body of the program. It gets the two input params
//  (src file and target directory), makes sure the source exists, creates
//  the output dir if needed), then tries to parse the src. If that succeeds
//  it creates an output generator and passes the parsed info to it to
//  spit out the output.
//
//  The output generator (who knows what the output files are for his 
//  particular language) will tell us if we need to actually do any output
//  or not. So we actually have to create that first before we parse.
//
#include "CQEnum_Pch.hpp"
#include "CQEnum.hpp"


using namespace CQSL;


// Used for any general 'bad parameter' type errors
static void ShowUsage(const char* const pszMsg)
{
    std::cout   << "\nCQEnum srcfile targetdir [options]\n\n    "
                << pszMsg
                << std::endl;
}


int main(int iArgs, char* apszArgs[])
{
    //
    //  We have to at least get the src file and target directory. For now, that's
    //  all there is, but other options may be added later.
    //
    if (iArgs < 3)
    {
        ShowUsage("You must provide the source file and target directory");
        return 1;
    }

    std::string strSrcFile(apszArgs[1]);
    std::string strTarDir(apszArgs[2]);
    if (!std::filesystem::exists(strSrcFile))
    {
        std::cout << "The source file was not found" << std::endl;
        return 1;
    }

    //
    //  If the target dir doesn't exist, we need to try to create it. If it does,
    //  it has to be a directory, not a file.
    //
    if (std::filesystem::exists(strTarDir))
    {
        if (!std::filesystem::is_directory(strTarDir))
        {
            std::cout << "The target must be a directory" << std::endl;
            return 1;            
        }
    }
    else
    {
        if (!std::filesystem::create_directories(strTarDir))
        {
            std::cout << "The target directory could not be created" << std::endl;
            return 1;            
        }
    }


    try
    {
        //
        //  Get the base name from the source file. This will be used to create
        //  any output files.
        //
        std::filesystem::path pathSrc(strSrcFile);
        std::filesystem::path pathBaseName = pathSrc.stem();

        //
        //  Seems a bit backwards, but only the output generator can check if the
        //  output is out of date wrt to the source file. So we create it first 
        //  and ask it if we need to do anything.
        //
        std::filesystem::file_time_type tmSrc = std::filesystem::last_write_time(strTarDir);
        CQEnum::CQEnumCppOutput gceoFmt;
        if (gceoFmt.bMustGenerate(strTarDir, pathBaseName.string(), tmSrc))
        {
            // He says do it, so open the source and parse into an info object
            CQEnum::InputSrc srcData;
            srcData.Open(strSrcFile);
            CQEnum::CQEnumInfo cqeiData;
            cqeiData.ParseFrom(srcData);

            // Ask the output generator to spit out the info appropriately
            gceoFmt.GenerateOutput(cqeiData, strTarDir, pathBaseName.string());
        }
    }

    catch(const std::exception& except)
    {
        std::cout   << "An error occurred while parsing the input file\n\n    "
                    << except.what()
                    << std::endl;
        return 1;
    }
    return 0;
}
