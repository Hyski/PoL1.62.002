//
// Shader Compiler methods
//
//============================================
//   FILE:  shdcomp.h   
//

#include "precomp.h"

#include "shader.h"

#include "tokens.h"
#include <pccts/AToken.h>
#include "ShdLexer.h"
#include "ShdRecorder.h"
#include "ShdCompiler.h"
#include "PakStream.h"
#include "OutWrapper.h"

const char* ShaderOutput::error_desc[] =
{    
    /*SC_SUCCESS,*/             "��� ������",
    /*SC_TOKBUFFER_ERR,*/       "������ ������ TokenBuffer",
    /*SC_TOKEN_ERR,*/           "������ ������ Token",
    /*SC_PARSER_ERR,*/          "��� ����� ������� syn ��� panic �������",
    /*SC_LEXER_ERR,*/           "������ �������",
	/*SC_FILE_OPEN_ERR,*/       "������ �������� �����",
    /*SC_NAME_REPLICATION,*/    "���������� �������� �������",
    /*SC_SECOND_CULL,*/         "������ �������� cull",
    /*SC_SECOND_ZBIAS,*/        "������ �������� zBias",
    /*SC_SECOND_ALPHAFUNC,*/    "������ ����������� alphaFunc",
    /*SC_SECOND_FILLMODE,*/     "������ �������� FillMode",
    /*SC_SECOND_RGBGEN,*/       "������ �������� rgbGen",
    /*SC_SECOND_ALPHAGEN,*/     "������ �������� alphaGen",
    /*SC_STAGE_NUM_ERR,*/       "������� ����� stages � �������",
    /*SC_NO_MAP_ERR,*/          "� stage'� ����������� �������� map | clampmap | animmap",
    /*SC_SECOND_MAP_ERR,*/      "��� �������� map(animmap)",
    /*SC_ANIM_FRAMES_ERR,*/     "������� ����� ������� � animmap",
    /*SC_SECOND_BLEND_ERR,*/    "������ �������� blendfunc",
    /*SC_SECOND_TCGEN_ERR,*/    "������ �������� tcGen",
    /*SC_TCMODS_FUNCS_ERR,*/    "������� ����� �������� tcMod",
    /*SC_SECOND_NOMIPMAP_ERR,*/ "������ �������� nomipmap",
    /*SC_SECOND_NOPICMIP_ERR,*/ "������ �������� nopicmip",
    /*SC_SECOND_STEPS_ERR,*/    "������ �������� steps",
    /*SC_SECOND_IMPACT_ERR,*/   "������ �������� impact",
    /*SC_SECOND_MARKS_ERR,*/    "������ �������� marks",
    /*SC_SECOND_DEPTHFUNC_ERR,*/"������ �������� depthFunc",
    /*SC_UNKNOWN_ERR,*/         "����������� ������",
};

ShdCompiler::ShdCompiler(ShaderOutput* p_out):
    p_output(p_out)
{
}

ShdCompiler::~ShdCompiler()
{
}

ShaderOutput* ShdCompiler::SetOutput(ShaderOutput* out)
{
    ShaderOutput* old_ptr = p_output;

    p_output = out;
    return old_ptr;
}

bool ShdCompiler::Compile(const char* file_name)
{
    DLGPakStream in(file_name);
    if(in.IsError()){
        p_output->OnError(file_name, 0, SC_FILE_OPEN_ERR);
        errors_count = 1;
        last_line = 0;

        return false;
    }

    //make shader parser
    ShdLexer            lexer(&in);
    ANTLRTokenBuffer    pipe(&lexer,1);
    FastToken           aToken;
    OutputWrapper       wrapper(file_name, p_output, &lexer);
    ShdRecorder         recorder(&pipe, &wrapper);

    lexer.setToken(&aToken);

    try{
        //compile file
        recorder.Run();
    }
    catch(ErrLimit)     { /*printf("\nerror limit\n");*/ }
    catch(LexerErr)     { wrapper.OnError(SC_LEXER_ERR); }

    catch(ParserErr)    { wrapper.OnError(SC_PARSER_ERR);}
    catch(TokenErr)     { wrapper.OnError(SC_TOKEN_ERR); }
    catch(TokBufferErr) { wrapper.OnError(SC_TOKBUFFER_ERR);}
    catch(PCCTS_Err)    { wrapper.OnError(SC_UNKNOWN_ERR);}
    
    //get last line & error count
    last_line    = lexer.line();
    errors_count = wrapper.GetErrorsCount();

    return (errors_count == 0);
}
