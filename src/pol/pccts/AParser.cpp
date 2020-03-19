/* ANTLRParser.C
 *
 * SOFTWARE RIGHTS
 *
 * We reserve no LEGAL rights to the Purdue Compiler Construction Tool
 * Set (PCCTS) -- PCCTS is in the public domain.  An individual or
 * company may do whatever they wish with source code distributed with
 * PCCTS or the code generated by PCCTS, including the incorporation of
 * PCCTS, or its output, into commerical software.
 *
 * We encourage users to develop software with PCCTS.  However, we do ask
 * that credit is given to us for developing PCCTS.  By "credit",
 * we mean that if you incorporate our source code into one of your
 * programs (commercial product, research project, or otherwise) that you
 * acknowledge this fact somewhere in the documentation, research report,
 * etc...  If you like PCCTS and have developed a nice tool with the
 * output, please mention that you developed it using PCCTS.  In
 * addition, we ask that this header remain intact in our source code.
 * As long as these guidelines are kept, we expect to continue enhancing
 * this system and expect to make other tools available as they are
 * completed.
 *
 * ANTLR 1.33
 * Terence Parr
 * Parr Research Corporation
 * with Purdue University and AHPCRC, University of Minnesota
 * 1989-1998
 */

#include "pcctscfg.h"

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

/* I have to put this here due to C++ limitation
 * that you can't have a 'forward' decl for enums.
 * I hate C++!!!!!!!!!!!!!!!
 * Of course, if I could use real templates, this would go away.
 */
// MR1
// MR1  10-Apr-97  133MR1  Prevent use of varying sizes for the
// MR1  			ANTLRTokenType enum
// MR1

enum ANTLRTokenType { TER_HATES_CPP=0, ITS_TOO_COMPLICATED=9999};	    // MR1

#define ANTLR_SUPPORT_CODE

#include ATOKEN_H
#include ATOKENBUFFER_H
#include APARSER_H

static const int zzINF_DEF_TOKEN_BUFFER_SIZE = 2000;    /* MR14 */
static const int zzINF_BUFFER_TOKEN_CHUNK_SIZE = 1000;  /* MR14 */

                 /* L o o k a h e a d  M a c r o s */

/* maximum of 32 bits/unsigned int and must be 8 bits/byte;
 * we only use 8 bits of it.
 */
SetWordType ANTLRParser::bitmask[sizeof(SetWordType)*8] = {
	0x00000001, 0x00000002, 0x00000004, 0x00000008,
	0x00000010, 0x00000020, 0x00000040, 0x00000080
};

char ANTLRParser::eMsgBuffer[500] = "";

ANTLRParser::
~ANTLRParser()
{
	delete [] token_type;
    delete [] zzFAILtext;       // MR16 Manfred Kogler
}

ANTLRParser::
ANTLRParser(ANTLRTokenBuffer *_inputTokens,
			int k,
			int use_inf_look,
			int dlook,
			int ssize)
{
	LLk = k;
	can_use_inf_look = use_inf_look;
/* MR14 */    if (dlook != 0) {
/* MR14 */      panic("ANTLRParser::ANTLRParser - Demand lookahead not supported in C++ mode");
/* MR14 */
/* MR14 */    };
    demand_look = 0;    /* demand_look = dlook; */
    bsetsize = ssize;
	guessing = 0;
	token_tbl = NULL;
	eofToken = (ANTLRTokenType)1;

	// allocate lookahead buffer
	token_type = new ANTLRTokenType[LLk];
	lap = 0;
	labase = 0;
#ifdef ZZDEFER_FETCH
	stillToFetch = 0;                                                   // MR19
#endif
	dirty = 0;
    inf_labase = 0;                                                     // MR7
    inf_last = 0;                                                       // MR7
	/* prime lookahead buffer, point to inputTokens */
	this->inputTokens = _inputTokens;
	this->inputTokens->setMinTokens(k);
	_inputTokens->setParser(this);					                    // MR1
    resynchConsumed=1;                                                  // MR8
    zzFAILtext=NULL;                                                    // MR9
    traceOptionValueDefault=0;                                          // MR10
    traceReset();                                                       // MR10
    zzGuessSeq=0;                                                       // MR10
    syntaxErrCount=0;                                                   // MR11
}

void ANTLRParser::init()
{
   prime_lookahead();
   resynchConsumed=1;                                                   // MR8
   traceReset();                                                        // MR10
}

void ANTLRParser::traceReset()
{
   traceOptionValue=traceOptionValueDefault;
   traceGuessOptionValue=1;
   traceCurrentRuleName=NULL;
   traceDepth=0;
}

int ANTLRParser::
guess(ANTLRParserState *st)
{
	saveState(st);
	guessing = 1;
	return setjmp(guess_start.state);
}

void ANTLRParser::
saveState(ANTLRParserState *buf)
{
	buf->guess_start = guess_start;
	buf->guessing = guessing;
	buf->inf_labase = inf_labase;
	buf->inf_last = inf_last;
	buf->dirty = dirty;
    buf->traceOptionValue=traceOptionValue;            /* MR10 */
    buf->traceGuessOptionValue=traceGuessOptionValue;  /* MR10 */
    buf->traceCurrentRuleName=traceCurrentRuleName;    /* MR10 */
    buf->traceDepth=traceDepth;                        /* MR10 */
}

void ANTLRParser::
restoreState(ANTLRParserState *buf)
{
	int     i;
    int     prevTraceOptionValue;

	guess_start = buf->guess_start;
	guessing = buf->guessing;
	inf_labase = buf->inf_labase;
	inf_last = buf->inf_last;
	dirty = buf->dirty;

	// restore lookahead buffer from k tokens before restored TokenBuffer position
	// if demand_look, then I guess we don't look backwards for these tokens.
	for (i=1; i<=LLk; i++) token_type[i-1] =
		inputTokens->bufferedToken(i-LLk)->getType();
	lap = 0;
	labase = 0;

    /* MR10 */

    prevTraceOptionValue=traceOptionValue;
    traceOptionValue=buf->traceOptionValue;
    if ( (prevTraceOptionValue > 0) !=
             (traceOptionValue > 0)) {
      if (traceOptionValue > 0) {
        throw ParserErr();
//Punch        fprintf(stderr,"trace enable restored in rule %s depth %d\n",traceCurrentRuleName,traceDepth);
      }
      if (traceOptionValue <= 0) {
        throw ParserErr();
//Punch        fprintf(stderr,"trace disable restored in rule %s depth %d\n",traceCurrentRuleName,traceDepth);
      }
    };
    traceGuessOptionValue=buf->traceGuessOptionValue;
    traceCurrentRuleName=buf->traceCurrentRuleName;
    traceDepth=buf->traceDepth;
    traceGuessDone(buf);
}

/* Get the next symbol from the input stream; put it into lookahead buffer;
 * fill token_type[] fast reference cache also.  NLA is the next place where
 * a lookahead ANTLRAbstractToken should go.
 */
void ANTLRParser::
consume()
{

#ifdef ZZDEBUG_CONSUME_ACTION
    zzdebug_consume_action();
#endif

// MR19 V.H. Simonis
//      Defer Fetch feature
//      Moves action of consume() into LA() function

#ifdef ZZDEFER_FETCH
      stillToFetch++;
#else
      NLA = inputTokens->getToken()->getType();
      dirty--;
      lap = (lap+1)&(LLk-1);
#endif

}

_ANTLRTokenPtr ANTLRParser::
LT(int i)
{

// MR19 V.H. Simonis
//      Defer Fetch feature
//      Moves action of consume() into LA() function

#ifdef ZZDEFER_FETCH
    undeferFetch();
#endif

#ifdef DEBUG_TOKENBUFFER
	if ( i >= inputTokens->bufferSize() || inputTokens->minTokens() < LLk )     /* MR20 Was "<=" */
	{
		char buf[2000];                 /* MR20 Was "static" */
        sprintf(buf, "The minimum number of tokens you requested that the\nANTLRTokenBuffer buffer is not enough to satisfy your\nLT(%d) request; increase 'k' argument to constructor for ANTLRTokenBuffer\n", i);
		panic(buf);
	}
#endif
	return inputTokens->bufferedToken(i-LLk);
}

void
ANTLRParser::
look(int k)
{
	int i, c = k - (LLk-dirty);
	for (i=1; i<=c; i++) consume();
}

/* fill the lookahead buffer up with k symbols (even if DEMAND_LOOK);
 */
void
ANTLRParser::
prime_lookahead()
{
	int i;
	for(i=1;i<=LLk; i++) consume();
	dirty=0;
	// lap = 0;     // MR14 Sinan Karasu (sinan.karasu@boeing.com)
	// labase = 0;  // MR14
    labase=lap;     // MR14
}

/* check to see if the current input symbol matches '_t'.
 * During NON demand lookahead mode, dirty will always be 0 and
 * hence the extra code for consuming tokens in _match is never
 * executed; the same routine can be used for both modes.
 */
int ANTLRParser::
_match(ANTLRTokenType _t, ANTLRChar **MissText,
	   ANTLRTokenType *MissTok, _ANTLRTokenPtr *BadTok,
	   SetWordType **MissSet)
{
	if ( dirty==LLk ) {
		consume();
	}
	if ( LA(1)!=_t ) {
		*MissText=NULL;
		*MissTok= _t; *BadTok = LT(1);
		*MissSet=NULL;
		return 0;
	}
	dirty++;
	labase = (labase+1)&(LLk-1);	// labase maintained even if !demand look
	return 1;
}

/* check to see if the current input symbol matches '_t'.
 * Used during exception handling.
 */
int ANTLRParser::
_match_wsig(ANTLRTokenType _t)
{
	if ( dirty==LLk ) {
		consume();
	}
	if ( LA(1)!=_t ) return 0;
	dirty++;
	labase = (labase+1)&(LLk-1);	// labase maintained even if !demand look
	return 1;
}

/* check to see if the current input symbol matches any token in a set.
 * During NON demand lookahead mode, dirty will always be 0 and
 * hence the extra code for consuming tokens in _match is never
 * executed; the same routine can be used for both modes.
 */
int ANTLRParser::
_setmatch(SetWordType *tset, ANTLRChar **MissText,
	   ANTLRTokenType *MissTok, _ANTLRTokenPtr *BadTok,
	   SetWordType **MissSet)
{
	if ( dirty==LLk ) {
		consume();
	}
	if ( !set_el(LA(1), tset) ) {
		*MissText=NULL;
		*MissTok= (ANTLRTokenType)0; *BadTok=LT(1);
		*MissSet=tset;
		return 0;
	}
	dirty++;
	labase = (labase+1)&(LLk-1);	// labase maintained even if !demand look
	return 1;
}

int ANTLRParser::
_setmatch_wsig(SetWordType *tset)
{
	if ( dirty==LLk ) {
		consume();
	}
	if ( !set_el(LA(1), tset) ) return 0;
	dirty++;
	labase = (labase+1)&(LLk-1);	// labase maintained even if !demand look
	return 1;
}

                   /* Exception handling routines */
//
//  7-Apr-97 133MR1
//   	     Change suggested by Eli Sternheim (eli@interhdl.com)
//
void ANTLRParser::
consumeUntil(SetWordType *st)
{
	ANTLRTokenType		tmp;	                        				// MR1
	const			int Eof=1;                                          // MR1
	while ( !set_el( (tmp=LA(1)), st) && tmp!=Eof) { consume(); }       // MR1
}

//
//  7-Apr-97 133MR1
//   	     Change suggested by Eli Sternheim (eli@interhdl.com)
//
void ANTLRParser::
consumeUntilToken(int t)
{
	int	tmp;                                                            // MR1
	const	int Eof=1;                                                  // MR1
	while ( (tmp=LA(1)) !=t && tmp!=Eof) { consume(); }                 // MR1
}


                        /* Old error stuff */

void ANTLRParser::
resynch(SetWordType *wd,SetWordType mask)
{

/* MR8              S.Bochnak@microtool.com.pl                          */
/* MR8              Change file scope static "consumed" to instance var */

	/* if you enter here without having consumed a token from last resynch
	 * force a token consumption.
	 */
/* MR8 */  	if ( !resynchConsumed ) {consume(); resynchConsumed=1; return;}

   	/* if current token is in resynch set, we've got what we wanted */

/* MR8 */  	if ( wd[LA(1)]&mask || LA(1) == eofToken ) {resynchConsumed=0; return;}
	
   	/* scan until we find something in the resynch set */

        	while ( !(wd[LA(1)]&mask) && LA(1) != eofToken ) {consume();}

/* MR8 */	resynchConsumed=1;
}

/* standard error reporting function that assumes DLG-based scanners;
 * you should redefine in subclass to change it or if you use your
 * own scanner.
 */
void ANTLRParser::
syn(_ANTLRTokenPtr tok, ANTLRChar *egroup, SetWordType *eset,
	ANTLRTokenType etok, int k)
{
	int line;

	line = LT(1)->getLine();

    syntaxErrCount++;                                   /* MR11 */
    throw ParserErr();
//Punch	fprintf(stderr, "line %d: syntax error at \"%s\"",
//Punch					line, LT(1)->getText());
//Punch	if ( !etok && !eset ) {
//Punch        fprintf(stderr, "\n"); 
//Punch        return;
//Punch    }
//Punch	if ( k==1 ) fprintf(stderr, " missing");
//Punch	else
//Punch	{
//Punch		fprintf(stderr, "; \"%s\" not", LT(1)->getText());
//Punch		if ( set_deg(eset)>1 ) fprintf(stderr, " in");
//Punch	}
//Punch	if ( set_deg(eset)>0 ) edecode(eset);
//Punch	else fprintf(stderr, " %s", token_tbl[etok]);
//Punch	if ( strlen(egroup) > 0 ) fprintf(stderr, " in %s", egroup);
//Punch	fprintf(stderr, "\n");
}

/* is b an element of set p? */
int ANTLRParser::
set_el(ANTLRTokenType b, SetWordType *p)
{
	return( p[DIVWORD(b)] & bitmask[MODWORD(b)] );
}

int ANTLRParser::
set_deg(SetWordType *a)
{
	/* Fast compute degree of a set... the number
	   of elements present in the set.  Assumes
	   that all word bits are used in the set
	*/
	register SetWordType *p = a;
	register SetWordType *endp = &(a[bsetsize]);
	register int degree = 0;

	if ( a == NULL ) return 0;
	while ( p < endp )
	{
		register SetWordType t = *p;
		register SetWordType *b = &(bitmask[0]);
		do {
			if (t & *b) ++degree;
		} while (++b < &(bitmask[sizeof(SetWordType)*8]));
		p++;
	}

	return(degree);
}

void ANTLRParser::
edecode(SetWordType *a)
{
	register SetWordType *p = a;
	register SetWordType *endp = &(p[bsetsize]);
	register unsigned e = 0;

    if ( set_deg(a)>1 ){
        throw ParserErr();
//Punch        fprintf(stderr, " {");
    }

	do {
		register SetWordType t = *p;
		register SetWordType *b = &(bitmask[0]);
		do {
            
            if ( t & *b ){
                 throw ParserErr();
//Punch                 fprintf(stderr, " %s", token_tbl[e]);
            }
			
            e++;
		} while (++b < &(bitmask[sizeof(SetWordType)*8]));
	} while (++p < endp);
	
    if ( set_deg(a)>1 ){
        throw ParserErr();
//Punch        fprintf(stderr, " }");
    }
}

/* input looks like:
 *      zzFAIL(k, e1, e2, ...,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk)
 * where the zzMiss stuff is set here to the token that did not match
 * (and which set wasn't it a member of).
 */

// MR9 29-Sep-97    Stan Bochnak (S.Bochnak@microTool.com.pl)
// MR9              Original fix to static allocated text didn't
// MR9                work because a pointer to it was passed back
// MR9                to caller.  Replace with instance variable.

const int   SETWORDCOUNT=20;

void
ANTLRParser::FAIL(int k, ...)
{
//
//  MR1 10-Apr-97	
//

    if (zzFAILtext == NULL) zzFAILtext=new char [1000];          // MR9
    SetWordType **f=new SetWordType *[SETWORDCOUNT];             // MR1 // MR9
    SetWordType **miss_set;
    ANTLRChar **miss_text;
    _ANTLRTokenPtr *bad_tok;
    ANTLRChar **bad_text;
//
//  7-Apr-97 133MR1
//  		err_k is passed as a "int *", not "unsigned *"
//
    int	*err_k;                                                         // MR1
    int i;
    va_list ap;

    va_start(ap, k);

    zzFAILtext[0] = '\0';
	if ( k > SETWORDCOUNT ) panic("FAIL: overflowed buffer");
    for (i=1; i<=k; i++)    /* collect all lookahead sets */
    {
        f[i-1] = va_arg(ap, SetWordType *);
    }
    for (i=1; i<=k; i++)    /* look for offending token */
    {
        if ( i>1 ) strcat(zzFAILtext, " ");
        strcat(zzFAILtext, LT(i)->getText());
        if ( !set_el(LA(i), f[i-1]) ) break;
    }
    miss_set = va_arg(ap, SetWordType **);
    miss_text = va_arg(ap, ANTLRChar **);
    bad_tok = va_arg(ap, _ANTLRTokenPtr *);
    bad_text = va_arg(ap, ANTLRChar **);
    err_k = va_arg(ap, int *);                      					// MR1
    if ( i>k )
    {
        /* bad; lookahead is permutation that cannot be matched,
         * but, the ith token of lookahead is valid at the ith position
         * (The old LL sub 1 (k) versus LL(k) parsing technique)
         */
        *miss_set = NULL;
        *miss_text = LT(1)->getText();
        *bad_tok = LT(1);
        *bad_text = (*bad_tok)->getText();
        *err_k = k;
//
//  MR4 20-May-97	erroneously deleted contents of f[]
//  MR4			        reported by Bruce Guenter (bruceg@qcc.sk.ca)
//  MR1 10-Apr-97	release temporary storage
//
      delete [] f;                                                      // MR1
      return;                                                           // MR1
    }
/*  fprintf(stderr, "%s not in %dth set\n", zztokens[LA(i)], i);*/
    *miss_set = f[i-1];
    *miss_text = zzFAILtext;
    *bad_tok = LT(i);
    *bad_text = (*bad_tok)->getText();
    if ( i==1 ) *err_k = 1;
    else *err_k = k;
//
//  MR4 20-May-97	erroneously deleted contents of f[]
//  MR4			      reported by Bruce Guenter (bruceg@qcc.sk.ca)
//  MR1 10-Apr-97	release temporary storage
//
    delete [] f;                                                        // MR1
    return;                                                             // MR1
}

int ANTLRParser::
_match_wdfltsig(ANTLRTokenType tokenWanted, SetWordType *whatFollows)
{
	if ( dirty==LLk ) consume();

	if ( LA(1)!=tokenWanted )
	{
        throw ParserErr();

//Punch        syntaxErrCount++;                                   /* MR11 */
//Punch		fprintf(stderr,
//Punch				"line %d: syntax error at \"%s\" missing %s\n",
//Punch				LT(1)->getLine(),
//Punch				(LA(1)==eofToken)?"<eof>":LT(1)->getText(),
//Punch				token_tbl[tokenWanted]);
//Punch		consumeUntil( whatFollows );
//Punch		return 0;
	}
	else {
		dirty++;
		labase = (labase+1)&(LLk-1); // labase maintained even if !demand look
/*		if ( !demand_look ) consume(); */
		return 1;
	}
}


int ANTLRParser::
_setmatch_wdfltsig(SetWordType *tokensWanted,
					ANTLRTokenType tokenTypeOfSet,
					SetWordType *whatFollows)
{
	if ( dirty==LLk ) consume();
	if ( !set_el(LA(1), tokensWanted) )
	{
        syntaxErrCount++;                                   /* MR11 */
	    throw ParserErr();
//Punch    	fprintf(stderr,
//Punch				"line %d: syntax error at \"%s\" missing %s\n",
//Punch				LT(1)->getLine(),
//Punch				(LA(1)==eofToken)?"<eof>":LT(1)->getText(),
//Punch				token_tbl[tokenTypeOfSet]);
//Punch		consumeUntil( whatFollows );
//Punch		return 0;
	}
	else {
		dirty++;
		labase = (labase+1)&(LLk-1); // labase maintained even if !demand look
/*		if ( !demand_look ) consume(); */
		return 1;
	}
}

char *ANTLRParser::
eMsgd(char *err,int d)
{
	sprintf(eMsgBuffer, err, d);	// dangerous, but I don't care
	return eMsgBuffer;
}

char *ANTLRParser::
eMsg(char *err, char *s)
{
	sprintf(eMsgBuffer, err, s);
	return eMsgBuffer;
}

char *ANTLRParser::
eMsg2(char *err,char *s, char *t)
{
	sprintf(eMsgBuffer, err, s, t);
	return eMsgBuffer;
}

void ANTLRParser::
panic(const char *msg)  // MR20 const
{
	throw ParserErr();

//Punch    fprintf(stderr, "ANTLR panic: %s\n", msg);
//Punch	exit(PCCTS_EXIT_FAILURE);           // MR1
}

const ANTLRChar *ANTLRParser::          // MR1
parserTokenName(int tok) {              // MR1
	return token_tbl[tok];              // MR1
}                                       // MR1

void ANTLRParser::traceGuessDone(const ANTLRParserState *state) {

  int   doIt=0;

  if (traceCurrentRuleName == NULL) return;

  if (traceOptionValue <= 0) {
    doIt=0;
  } else if (traceGuessOptionValue <= 0) {
    doIt=0;
  } else {
    doIt=1;
  };

  if (doIt) {

    throw ParserErr();
//Punch    fprintf(stderr,"guess done - returning to rule %s {\"%s\"} at depth %d",
//Punch        state->traceCurrentRuleName,
//Punch        LT(1)->getType() == eofToken ? "@" : LT(1)->getText(),
//Punch        state->traceDepth);
//Punch    if (state->guessing != 0) {
//Punch      fprintf(stderr," (guess mode continues - an enclosing guess is still active)");
//Punch    } else {
//Punch      fprintf(stderr," (guess mode ends)");
//Punch    };
//Punch    fprintf(stderr,"\n");
  };
}

void ANTLRParser::traceGuessFail() {

  int   doIt=0;

  if (traceOptionValue <= 0) {
    doIt=0;
  } else if (guessing && traceGuessOptionValue <= 0) {
    doIt=0;
  } else {
    doIt=1;
  };

  if (doIt) {
              
    throw ParserErr();

//Punch    fprintf(stderr,"guess failed\n");
  };
}

/* traceOption:
     zero value turns off trace
*/

void ANTLRParser::tracein(const ANTLRChar * rule) {

  int       doIt=0;

  traceDepth++;
  traceCurrentRuleName=rule;

  if (traceOptionValue <= 0) {
    doIt=0;
  } else if (guessing && traceGuessOptionValue <= 0) {
    doIt=0;
  } else {
    doIt=1;
  };

  if (doIt) {
            
    throw ParserErr();

//Punch    fprintf(stderr,"enter rule %s {\"%s\"} depth %d",
//Punch            rule,
//Punch            LT(1)->getType() == eofToken ? "@" : LT(1)->getText(),
//Punch            traceDepth);
//Punch    if (guessing) fprintf(stderr," guessing");
//Punch    fprintf(stderr,"\n");
  };
  return;
}

void ANTLRParser::traceout(const ANTLRChar * rule) {

  int       doIt=0;

  traceDepth--;

  if (traceOptionValue <= 0) {
    doIt=0;
  } else if (guessing && traceGuessOptionValue <= 0) {
    doIt=0;
  } else {
    doIt=1;
  };

  if (doIt) {
            
    throw ParserErr();

//Punch    fprintf(stderr,"exit rule %s {\"%s\"} depth %d",
//Punch            rule,
//Punch            LT(1)->getType() == eofToken ? "@" : LT(1)->getText(),
//Punch            traceDepth+1);
//Punch    if (guessing) fprintf(stderr," guessing");
//Punch    fprintf(stderr,"\n");
  };
}

int ANTLRParser::traceOption(int delta) {

    int     prevValue=traceOptionValue;

    traceOptionValue=traceOptionValue+delta;

    if (traceCurrentRuleName != NULL) {
      if (prevValue <= 0 && traceOptionValue > 0) {
                  
        throw ParserErr();

//Punch        fprintf(stderr,"trace enabled in rule %s depth %d\n",traceCurrentRuleName,traceDepth);
      };
      if (prevValue > 0 && traceOptionValue <= 0) {
                  
        throw ParserErr();

//Punch        fprintf(stderr,"trace disabled in rule %s depth %d\n",traceCurrentRuleName,traceDepth);
      };
    };

    return  prevValue;
}

int ANTLRParser::traceGuessOption(int delta) {

    int     prevValue=traceGuessOptionValue;

    traceGuessOptionValue=traceGuessOptionValue+delta;

    if (traceCurrentRuleName != NULL) {
      if (prevValue <= 0 && traceGuessOptionValue > 0) {
                  
        throw ParserErr();

//Punch        fprintf(stderr,"guess trace enabled in rule %s depth %d\n",traceCurrentRuleName,traceDepth);
      };
      if (prevValue > 0 && traceGuessOptionValue <= 0) {
                  
        throw ParserErr();

//Punch        fprintf(stderr,"guess trace disabled in rule %s depth %d\n",traceCurrentRuleName,traceDepth);
      };
    };
    return prevValue;
}

// MR19 V.H. Simonis Defer Fetch feature

void ANTLRParser::undeferFetch()
{

#ifdef ZZDEFER_FETCH
    if (stillToFetch) {
        for (int stillToFetch_x = 0; stillToFetch_x < stillToFetch; ++stillToFetch_x) {
    		NLA = inputTokens->getToken()->getType();
    		dirty--;
    		lap = (lap+1)&(LLk-1);
        }
        stillToFetch = 0;
    }
#else
    return;
#endif

}

int ANTLRParser::isDeferFetchEnabled()
{
#ifdef ZZDEFER_FETCH
    return 1;
#else
    return 0;
#endif
}
