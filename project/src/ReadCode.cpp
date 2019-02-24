#include "Component.h"
#include "Configuration.h"
#include "File.h"
#include "Project.h"
#include "known.h"

#include <algorithm>
#include <fcntl.h>
#include <fw/filesystem.hpp>

void Project::ReadCodeFrom(File &f, const char *buffer, size_t buffersize)
{
    size_t start = 0;
    bool pointyBrackets = true;
    bool exported = false;
    enum State
    {
        None,
        Dirty,
        AfterHash,
        AfterImport,
        AfterInclude,
        AfterModule
    } state = None;
    for(size_t offset = 0; offset < buffersize; offset++)
    {
        switch(state)
        {
        case None:
            switch(buffer[offset]) 
            {
            case '#':
                state = AfterHash;
                break;
            case '/': // Check for and skip comment blocks
                if(buffer[offset + 1] == '/')
                {
                    offset = static_cast<const char *>(memchr(buffer + offset, '\n', buffersize - offset)) - buffer;
                }
                else if(buffer[offset + 1] == '*')
                {
                    do
                    {
                        const char *endSlash = static_cast<const char *>(memchr(buffer + offset + 1, '/', buffersize - offset));
                        if(!endSlash)
                            return;
                        offset = endSlash - buffer;
                    } while(buffer[offset - 1] != '*');
                }
                break;
            case ' ':
            case '\t':
            case '\n':
            case '\r':
            case '\f':
                break;
            case 'e':
                if(buffer[offset + 1] == 'x' && buffer[offset + 2] == 'p' && buffer[offset + 3] == 'o' && buffer[offset + 4] == 'r' && buffer[offset + 5] == 't' && isspace(buffer[offset+6]))
                {
                    exported = true;
                    offset += 5;
                }
                else
                {
                    state = Dirty;
                }
                break;
            case 'i':
                if(buffer[offset + 1] == 'm' && buffer[offset + 2] == 'p' && buffer[offset + 3] == 'o' && buffer[offset + 4] == 'r' && buffer[offset + 5] == 't' && (isspace(buffer[offset+6]) || buffer[offset+6] == '<' || buffer[offset+6] == '"'))
                {
                    state = AfterImport;
                    offset += 5;
                }
                else
                {
                    state = Dirty;
                }
                break;
            case 'm':
                if(buffer[offset + 1] == 'o' && buffer[offset + 2] == 'd' && buffer[offset + 3] == 'u' && buffer[offset + 4] == 'l' && buffer[offset + 5] == 'e' && isspace(buffer[offset+6]))
                {
                    state = AfterModule;
                    offset += 5;
                }
                else
                {
                    state = Dirty;
                }
                break;

            default:
                state = Dirty;
                break;
            }
            break;
        case Dirty:
            switch(buffer[offset]) 
            {
            case '#':
                state = AfterHash;
                break;
            case ';':
                state = None;
                break;
            case '/': // Check for and skip comment blocks
                if(buffer[offset + 1] == '/')
                {
                    offset = static_cast<const char *>(memchr(buffer + offset, '\n', buffersize - offset)) - buffer;
                }
                else if(buffer[offset + 1] == '*')
                {
                    do
                    {
                        const char *endSlash = static_cast<const char *>(memchr(buffer + offset + 1, '/', buffersize - offset));
                        if(!endSlash)
                            return;
                        offset = endSlash - buffer;
                    } while(buffer[offset - 1] != '*');
                }
                break;
            }
            break;
        case AfterHash:
            switch(buffer[offset])
            {
            case ' ':
            case '\t':
                break;
            case 'i':
                if(buffer[offset + 1] == 'm' && buffer[offset + 2] == 'p' && buffer[offset + 3] == 'o' && buffer[offset + 4] == 'r' && buffer[offset + 5] == 't' && (isspace(buffer[offset+6]) || buffer[offset+6] == '<' || buffer[offset+6] == '"'))
                {
                    state = AfterImport;
                    offset += 5;
                }
                else if(buffer[offset + 1] == 'n' && buffer[offset + 2] == 'c' && buffer[offset + 3] == 'l' && buffer[offset + 4] == 'u' && buffer[offset + 5] == 'd' && buffer[offset + 6] == 'e' && (isspace(buffer[offset+7]) || buffer[offset+7] == '<' || buffer[offset+7] == '"'))
                {
                    state = AfterInclude;
                    offset += 6;
                }
                else
                {
                    state = None;
                }
                break;
            default:
                state = None;
                break;
            }
            break;
        case AfterImport:
        case AfterInclude:
        case AfterModule:
            switch(buffer[offset])
            {
            case ' ':
            case '\t':
                break;

            case '<':
            case '"':
                pointyBrackets = (buffer[offset] == '<');
                offset++;
                start = offset;
                while(state != None && offset < buffersize)
                {
                    switch(buffer[offset])
                    {
                    case ';':
                    case '\n':
                        state = None; // Buggy code, skip over this include.
                        break;
                    case '>':
                    case '\"':
                        // Yes, we'll match a mismatched pair. That's fine.
                        if(state == AfterInclude)
                            f.AddIncludeStmt(pointyBrackets, std::string(&buffer[start], &buffer[offset]));
                        else
                            f.AddImportStmt(pointyBrackets, std::string(&buffer[start], &buffer[offset]));
                        state = None;
                        break;
                    }
                    offset++;
                }
                break;
            default:
                if(isalnum(buffer[offset]) || buffer[offset] == '_' || buffer[offset] == ':')
                {
                    std::string modulename;
                    while(buffer[offset] != ';' && buffer[offset] != '[' && buffer[offset] != '\n')
                    {
                        if(!isspace(buffer[offset]))
                            modulename += buffer[offset];
                        offset++;
                    }
                    if(buffer[offset] != '\n')
                    {
                        if(state == AfterModule)
                        {
                            printf("%s\n", modulename.c_str());
                            f.SetModule(modulename, exported);
                        }
                        else
                        {
                            printf("%s\n", modulename.c_str());
                            f.AddImport(modulename, exported);
                        }
                        exported = false;
                    }
                }
                if (buffer[offset] == ';')
                    state = None;
                else
                    state = Dirty;
                break;
            }
            break;
        }
    }
}
