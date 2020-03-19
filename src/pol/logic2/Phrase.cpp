#include "logicdefs.h"

#include "Phrase.h"
#include "SndUtils.h"

//=====================================================================================//
//                                phrase_s::phrase_s()                                 //
//=====================================================================================//
phrase_s::phrase_s(const std::string& str4parse)
{
    const char* separator = "##";

    size_t  beg = 0, 
            end = 0, 
            sep_size = strlen(separator);
    
    std::string tmp;
    std::vector<std::string> vec;
    
    while(true)
	{
        end = str4parse.find(separator, beg, sep_size);
        
        if(beg != end)
		{
            vec.push_back(std::string());
            vec.back().assign(str4parse, beg, end-beg);
            
            tmp = vec.back();
        }
        
        if(end == std::string::npos)
		{
            break;
		}
        
        beg = end + sep_size;
    }
    
    if(vec.empty()) return;
    
    std::istringstream istr(vec[aiRangeRand(vec.size())]);
    tmp = istr.str();
    
    while(true)
	{
        char ch = istr.get();
        
        if(!istr.good())
		{
            break;
		}
        
        if(ch == '$')
		{
            std::streampos pos = istr.tellg();
            
            istr >> ch;
            
            if(ch == '(')
			{
                tmp.clear();
                
                while(true)
				{
                    ch = istr.get();
                    
                    if(!istr.good() || ch == ')')
					{
                        break;                                                        
					}
                    
                    tmp += ch;
                }
            }
            
            if(ch == ')')
			{
                m_sound = SndUtils::MakeSoundFileName(tmp);
                continue;
            }
			else
			{
                istr.clear();
                istr.seekg(pos);
            }
        }
        
        m_text += ch;
    }
}
