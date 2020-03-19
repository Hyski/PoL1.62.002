header "pre_include_hpp"
{
#include "QuestParams.h"
#include "QuestMessage.h"
#include "LogicalCondition.h"
#include "QuestConditionState.h"
}

header "pre_include_cpp"
{
#include "precomp.h"
#include <boost/lexical_cast.hpp>
#include "Task.h"
#include "Quest.h"
#include "Trigger.h"
#include "Counter.h"
#include "Condition.h"
#include "Persistent.h"
#include "ActFactory.h"
#include "ActTemplate.h"
}

options
{
	language="Cpp";
	genHashLines=false;
}

//=====================================================================================//
//                       class ActScriptParser extends Parser;                         //
//=====================================================================================//
class ActScriptParser extends Parser;
options
{
	k=3;
	buildAST = true;
}
tokens
{
	ACT_DEFINE; ACT_BODY;
	PARALLEL_ACT; SEQUENTIAL_ACT;
	WALK_ACT; WALKZ_ACT;
	RUN_ACT; RUNZ_ACT;
	CRAWL_ACT; CRAWLZ_ACT;
	LEVEL_ACT; WINGAME_ACT; LOOSEGAME_ACT;
	WAIT_ACT; QSEND_ACT; SSCENE_ACT;
	INC_ACT; DEC_ACT; CAMFOCUS_ACT;
	ENABLECOUNTER_ACT; DISABLECOUNTER_ACT;
	REF_ACT; EXP_ACT; MONEY_ACT; REPORT_ACT; PULSE_ACT;
	GIVE_ACT; ADD_ACT; RESET_ACT; ADDRANK_ACT;
	ADDHERO_ACT; REMOVEHERO_ACT; PHASE_ACT;
	FORCEADDHERO_ACT;
	ENABLELEVEL_ACT; DISABLELEVEL_ACT;
	DISAPPEAR_ACT; CAMSET_ACT; CAMSPLINE_ACT;
	MOVIE_ACT; SPAWN_ACT; DESTRUCT_ACT;
	SHOOT_ACT; DIE_ACT; KILL_ACT;
	ADDRECORD_ACT; ERASERECORD_ACT;
	SAY_ACT; OPEN_ACT; CLOSE_ACT;
	ADDNEWS_ACT; CAMFOLLOW_ACT;
	ALIGNMENT_ACT;

	TASK_DEFINE; TASK_BODY;
	BRING_TASK; BRING2_TASK; KILL_TASK;
	DEAD_TASK; STUB_TASK;
	REF_TASK; ALIVE_TASK;
	TALK_TASK; TALK2_TASK;
	ELIMINATE_TASK;
	
	COND_DEFINE;
	COND_BODY;
	OR_COND; AND_COND;
	COMPLETED_QUEST_COND; FAILED_QUEST_COND; STARTED_QUEST_COND; PASSED_QUEST_COND; NOTSTARTED_QUEST_COND;
	GREATER_COND; LESS_COND; EQUAL_COND; LESSEQUAL_COND; GREATEREQUAL_COND; NOTEQUAL_COND;
	REF_COND; HAVEHERO_COND; NOHERO_COND; EPISODE_COND; CONTEXT_COND;
	COMPLETED_TASK_COND; FAILED_TASK_COND; NOTCOMPLETED_TASK_COND;
	ALIVE_COND; DEAD_COND;
	
	TRIGGER_DEFINE; STRIGGER_DEFINE;
	
	QUEST_DEFINE;
	QUEST_TASK_DEFINE; QUEST_START_TASK_DEFINE; QUEST_END_TASK_DEFINE; QUEST_REMIND_TASK_DEFINE;
	QUEST_DISABLED; QUEST_HOLDER_DEFINE;
	QUEST_ON_START_ACT; QUEST_ON_SUCCEED_ACT; QUEST_ON_FAIL_ACT; QUEST_ON_REMIND_ACT;
	QUEST_ON_COMPLETED_ON_START_ACT; QUEST_ON_FAILED_ON_START_ACT; QUEST_ON_PRE_SUCCEED;
	QUEST_DEPENDENCY;
	
	
	COUNTER_DEFINE;
	INC_COUNTER; DEC_COUNTER; COUNTER_DISABLED;
	
	SIGNAL_DEFINE;
	SIGNAL_BODY;
	OR_SIGNAL; ONKILL_SIGNAL; ONDEAD_SIGNAL; ONUSE_SIGNAL; ONTALK_SIGNAL; ONSTURN_SIGNAL; ONETURN_SIGNAL;
	ONLEAVELEVEL_SIGNAL;
	
	FLOATNODE;
}
{

void reportError(const antlr::RecognitionException& ex)
{
	ACT_LOG("ERROR: " << ex.toString().c_str() << "\n");
}

void reportError(const std::string& s)
{
	if (getFilename()=="")
	{
		ACT_LOG("ERROR: " << s.c_str() << "\n");
	}
	else
	{
		ACT_LOG("ERROR: " << getFilename().c_str() << ": " << s.c_str() << "\n");
	}
}

void reportWarning(const std::string& s)
{
	if (getFilename()=="")
	{
		ACT_LOG("WARNING: " << s.c_str() << "\n");
	}
	else
	{
		ACT_LOG("WARNING: " << getFilename().c_str() << ": " << s.c_str() << "\n");
	}
}

}
debug_program	:	{
						while(LT(1)->getType() != EOF_)
						{
							ACT_LOG(LT(1)->getText() << " - ["
									  << getTokenName(LT(1)->getType()) << "]\n");
							consume();
						}
					}
				;

program			:	(act|task|trigger|quest|condition|counter)* EOF!
				;
				
counter			:	TKN_COUNTER^ { #TKN_COUNTER->setType(COUNTER_DEFINE); }
					(counterDisabled)?
					identifier (EQUALS! integer)?
					counterProperties
					(DOT!|SEMICOLON!)
				;

counterDisabled	:	TKN_DISABLED { #TKN_DISABLED->setType(COUNTER_DISABLED); }
				;
				
counterProperties
				{
					bool hasInc = false, hasDec = false;
				}
				:	(	{!hasInc}?	incCounterSignal	{ hasInc = true; }
					|	{!hasDec}?	decCounterSignal	{ hasDec = true; }
					)*
				;
				
incCounterSignal:	TKN_INC^ { #TKN_INC->setType(INC_COUNTER); }
					COLON! conditionBody (SEMICOLON!|DOT!)
				;

decCounterSignal:	TKN_DEC^ { #TKN_DEC->setType(DEC_COUNTER); }
					COLON! conditionBody (SEMICOLON!|DOT!)
				;
				
condition		:	TKN_CONDITION^ { #TKN_CONDITION->setType(COND_DEFINE); }
					identifier EQUALS! conditionBody (SEMICOLON!|DOT!)
				;
				
quest			:	TKN_QUEST^ { #TKN_QUEST->setType(QUEST_DEFINE); }
					identifier (questDisabled)? EQUALS! questProperties (SEMICOLON!|DOT!)
				;
				
questDisabled	:	TKN_DISABLED { #TKN_DISABLED->setType(QUEST_DISABLED); }
				;
				
questProperties	{
					bool hasStartTask = false, hasEndTask = false, hasTask = false, hasHolder = false;
					bool hasRemindTask = false;
					bool hasOnStart = false, hasOnSucceed = false, hasOnFail = false, hasOnRemind = false;
					bool hasOnComplOnStart = false, hasOnFailOnStart = false;
					bool hasDependency = false, hasOnPreSucceed = false;
				}
				:	(	{!hasTask}?						questTask			{ hasTask = true; }
					|	{!hasStartTask}?				questStartTask		{ hasStartTask = true; }
					|	{!hasEndTask}?					questEndTask		{ hasEndTask = true; }
					|	{!hasHolder}?					questHolder			{ hasHolder = true; }
					|	{!hasRemindTask}?				questRemindTask		{ hasRemindTask = true; }
					|	{!hasOnStart}?					questOnStart		{ hasOnStart = true; }
					|	{!hasOnSucceed}?				questOnSucceed		{ hasOnSucceed = true; }
					|	{!hasOnFail}?					questOnFail			{ hasOnFail = true; }
					|	{!hasOnRemind}?					questOnRemind		{ hasOnRemind = true; }
					|	{!hasOnComplOnStart}?			questOnComplOnStart	{ hasOnComplOnStart = true; }
					|	{!hasOnFailOnStart}?			questOnFailOnStart	{ hasOnFailOnStart = true; }
					|	{!hasDependency}?				questDependency		{ hasDependency = true; }
					|	{!hasOnPreSucceed}?				questOnPreSucceed	{ hasOnPreSucceed = true; }
					)*
				;
				
questOnPreSucceed
				:	TKN_ON! TKN_PRE_SUCCEED^ { #TKN_PRE_SUCCEED->setType(QUEST_ON_PRE_SUCCEED); }
					COLON! actBody (SEMICOLON!|DOT!)
				;
				
questDependency	:	TKN_DEPENDSON^ { #TKN_DEPENDSON->setType(QUEST_DEPENDENCY); }
					COLON! conditionBody
					(SEMICOLON!|DOT!)
				;			
	
questHolder		:	TKN_QUESTHOLDER^ { #TKN_QUESTHOLDER->setType(QUEST_HOLDER_DEFINE); }
					COLON! identifier (SEMICOLON!|DOT!)
				;
				
questOnComplOnStart
				:	TKN_ON!
					(	TKN_COMPLONSTART^ { #TKN_COMPLONSTART->setType(QUEST_ON_COMPLETED_ON_START_ACT); }
					|	TKN_COMPLONS^ { #TKN_COMPLONS->setType(QUEST_ON_COMPLETED_ON_START_ACT); }
					)
					COLON! actBody (SEMICOLON!|DOT!)
				;

questOnFailOnStart
				:	TKN_ON!
					(	TKN_FAILONSTART^ { #TKN_FAILONSTART->setType(QUEST_ON_FAILED_ON_START_ACT); }
					|	TKN_FAILONS^ { #TKN_FAILONS->setType(QUEST_ON_FAILED_ON_START_ACT); }
					)
					COLON! actBody (SEMICOLON!|DOT!)
				;
				
questOnStart	:	TKN_ON! TKN_START^ { #TKN_START->setType(QUEST_ON_START_ACT); }
					COLON! actBody (SEMICOLON!|DOT!)
				;
				
questOnSucceed	:	TKN_ON! TKN_SUCCEED^ { #TKN_SUCCEED->setType(QUEST_ON_SUCCEED_ACT); }
					COLON! actBody (SEMICOLON!|DOT!)
				;

questOnFail		:	TKN_ON! TKN_FAIL^ { #TKN_FAIL->setType(QUEST_ON_FAIL_ACT); }
					COLON! actBody (SEMICOLON!|DOT!)
				;

questOnRemind	:	TKN_ON! TKN_REMIND^ { #TKN_REMIND->setType(QUEST_ON_REMIND_ACT); }
					COLON! actBody (SEMICOLON!|DOT!)
				;

questTask		:	TKN_TASK^ { #TKN_TASK->setType(QUEST_TASK_DEFINE); }
					COLON! taskBody (SEMICOLON!|DOT!)
				;

questRemindTask	:	TKN_REMIND^ { #TKN_REMIND->setType(QUEST_REMIND_TASK_DEFINE); }
					COLON! conditionBody (SEMICOLON!|DOT!)
				;

questStartTask	:	TKN_STARTTASK^ { #TKN_STARTTASK->setType(QUEST_START_TASK_DEFINE); }
					COLON! taskBody (SEMICOLON!|DOT!)
				;

questEndTask	:	TKN_ENDTASK^ { #TKN_ENDTASK->setType(QUEST_END_TASK_DEFINE); }
					COLON! taskBody (SEMICOLON!|DOT!)
				;
				
trigger			:	(	TKN_TRIGGER^ { #TKN_TRIGGER->setType(TRIGGER_DEFINE); }
					|	TKN_STRIGGER^ { #TKN_STRIGGER->setType(STRIGGER_DEFINE); }
					)
					identifier EQUALS! conditionBody COLON! actBody
					(SEMICOLON!|DOT!)
				;
				
conditionBody	:	orCondition { #conditionBody = #([COND_BODY],#conditionBody); }
				;
				
orCondition		:	andCondition
					(
						PARALLEL^ { #PARALLEL->setType(OR_COND); }
						orCondition
					)?
				;
				
andCondition	:	primaryCondition
					(
						SEQUENTIAL^ { #SEQUENTIAL->setType(AND_COND); }
						andCondition
					)?
				;
				
primaryCondition:	concreteCondition
				|	LBRACE! orCondition RBRACE!
				;
				
concreteCondition:	completedCondition
				|	notCompletedCondition
				|	failedCondition
				|	startedCondition
				|	notStartedCondition
				|	passedCondition
				|	refConditionOrCompareCondition
				|	onKillSignal
				|	onDeadSignal
				|	onTalkSignal
				|	onUseSignal
				|	onStartTurnSignal
				|	onEndTurnSignal
				|	episodeCondition
				|	haveHeroCondition
				|	noHeroCondition
				|	onLeaveLevelSignal
				|	contextCondition
				|	aliveCondition
				|	deadCondition
				;
				
deadCondition	:	TKN_DEAD^ { #TKN_DEAD->setType(DEAD_COND); }
					LBRACE! identifier RBRACE!
				;
				
aliveCondition	:	TKN_ALIVE^ { #TKN_ALIVE->setType(ALIVE_COND); }
					LBRACE! identifier RBRACE!
				;
				
contextCondition:	TKN_CONTEXT^ { #TKN_CONTEXT->setType(CONTEXT_COND); }
					LBRACE! identifier (COMMA! integer)? RBRACE!
				;
				
onLeaveLevelSignal
				:	TKN_ONLEAVELEVEL^ { #TKN_ONLEAVELEVEL->setType(ONLEAVELEVEL_SIGNAL); }
					LBRACE! identifier (COMMA! identifier)? RBRACE!
				;
				
episodeCondition:	TKN_EPISODE^ { #TKN_EPISODE->setType(EPISODE_COND); }
					LBRACE! integer RBRACE!
				;
				
haveHeroCondition:	TKN_HAVEHERO^ { #TKN_HAVEHERO->setType(HAVEHERO_COND); }
					LBRACE! identifier RBRACE!
				;

noHeroCondition	:	TKN_NOHERO^ { #TKN_NOHERO->setType(NOHERO_COND); }
					LBRACE! identifier RBRACE!
				;

refConditionOrCompareCondition
				:	((IDENTIFIER|INTEGER) (GREATER|LESS|EQUALS|GREATEREQUAL|LESSEQUAL|NOTEQUAL) )=> compareCondition
				|	refCondition
				;
				
refCounter		:	identifier { #refCounter = #([REF_COUNTER],#refCounter); }
				;

compareCondition:	(refCounter|integer)
					(	GREATER^		{ #GREATER->setType(GREATER_COND); }
					|	LESS^			{ #LESS->setType(LESS_COND); }
					|	EQUALS^			{ #EQUALS->setType(EQUAL_COND); }
					|	GREATEREQUAL^	{ #GREATEREQUAL->setType(GREATEREQUAL_COND); }
					|	LESSEQUAL^		{ #LESSEQUAL->setType(LESSEQUAL_COND); }
					|	NOTEQUAL^		{ #NOTEQUAL->setType(NOTEQUAL_COND); }
					)
					(refCounter|integer)
				;

onKillSignal	:	TKN_ONKILL^ { #TKN_ONKILL->setType(ONKILL_SIGNAL); }
					LBRACE! identifier RBRACE!
				;
				
onDeadSignal	:	TKN_ONDEAD^ { #TKN_ONDEAD->setType(ONDEAD_SIGNAL); }
					LBRACE! identifier RBRACE!
				;
onTalkSignal	:	TKN_ONTALK^ { #TKN_ONTALK->setType(ONTALK_SIGNAL); }
					LBRACE! identifier RBRACE!
				;
onUseSignal		:	TKN_ONUSE^ { #TKN_ONUSE->setType(ONUSE_SIGNAL); }
					LBRACE! identifier (COMMA! identifier)? RBRACE!
				;
				
onStartTurnSignal
				:	TKN_ONSTURN^ { #TKN_ONSTURN->setType(ONSTURN_SIGNAL); }
				;

onEndTurnSignal	:	TKN_ONETURN^ { #TKN_ONETURN->setType(ONETURN_SIGNAL); }
				;
				
completedCondition:	a:TKN_COMPLETED^ { #a->setType(COMPLETED_QUEST_COND); }
					LBRACE!
						(	identifier
						|	taskBody2
							{
								#a->setType(COMPLETED_TASK_COND);
							}
						)
					RBRACE!
				;

failedCondition	:	a:TKN_FAILED^ { #a->setType(FAILED_QUEST_COND); }
					LBRACE!
						(	identifier
						|	taskBody2
							{
								#a->setType(FAILED_TASK_COND);
							}
						)
					RBRACE!
				;

notCompletedCondition:	TKN_NOTCOMPLETED^ { #TKN_NOTCOMPLETED->setType(NOTCOMPLETED_TASK_COND); }
					LBRACE!	taskBody RBRACE!
				;

passedCondition	:	TKN_PASSED^ { #TKN_PASSED->setType(PASSED_QUEST_COND); }
					LBRACE! identifier RBRACE!
				;

startedCondition:	TKN_STARTED^ { #TKN_STARTED->setType(STARTED_QUEST_COND); }
					LBRACE! identifier RBRACE!
				;

notStartedCondition
				:	TKN_NOTSTARTED^ { #TKN_NOTSTARTED->setType(NOTSTARTED_QUEST_COND); }
					LBRACE! identifier RBRACE!
				;

refCondition	:	identifier { #refCondition = #([REF_COND],#refCondition); }
				;
				
task			:	TKN_TASK! (identifier)? EQUALS^ { #EQUALS->setType(TASK_DEFINE); } taskBody (SEMICOLON!|DOT!)
				;
				
taskBody		:	taskSequence { #taskBody = #([TASK_BODY],#taskBody); }
				;
				
taskBody2		:	taskSequence2 { #taskBody2 = #([TASK_BODY],#taskBody2); }
				;
				
taskSequence	:	(concreteTask)*
				;

taskSequence2	:	(concreteTask2)*
				;

concreteTask2	:	killTask
				|	deadTask
				|	bringTask
				|	bring2Task
				|	stubTask
				|	aliveTask
				|	talkTask
				|	talk2Task
				|	eliminateTask
				;
				
concreteTask	:	killTask
				|	deadTask
				|	bringTask
				|	bring2Task
				|	stubTask
				|	taskReference
				|	aliveTask
				|	talkTask
				|	talk2Task
				|	eliminateTask
				;
				
eliminateTask	:	TKN_ELIMINATE^ { #TKN_ELIMINATE->setType(ELIMINATE_TASK); }
					LBRACE! identifier COMMA! identifier RBRACE!
				;
				
killTask		:	TKN_KILL^ { #TKN_KILL->setType(KILL_TASK); }
					LBRACE! identifier RBRACE!
				;
				
deadTask		:	TKN_DEAD^ { #TKN_DEAD->setType(DEAD_TASK); }
					LBRACE! identifier RBRACE!
				;
				
bringTask		:	TKN_BRING^ { #TKN_BRING->setType(BRING_TASK); }
					LBRACE! identifier COMMA! identifier RBRACE!
				;

bring2Task		:	TKN_BRING2^ { #TKN_BRING2->setType(BRING2_TASK); }
					LBRACE! identifier COMMA! identifier RBRACE!
				;
				
stubTask		:	TKN_STUB^ { #TKN_STUB->setType(STUB_TASK); }
				;
				
taskReference	:	identifier { #taskReference = #([REF_TASK],#taskReference); }
				;
				
aliveTask		:	TKN_ALIVE^ { #TKN_ALIVE->setType(ALIVE_TASK); }
					LBRACE! identifier RBRACE!
				;
				
talkTask		:	TKN_TALK^ { #TKN_TALK->setType(TALK_TASK); }
					LBRACE! identifier RBRACE!
				;

talk2Task		:	TKN_TALK2^ { #TKN_TALK2->setType(TALK2_TASK); }
					LBRACE! identifier RBRACE!
				;
				
act				:	identifier EQUALS^ { #EQUALS->setType(ACT_DEFINE); } actBody (SEMICOLON!|DOT!)
				;
				
actBody			:	parallelAct { #actBody = #([ACT_BODY], #actBody); }
				;
				
parallelAct		:	sequentialAct
					(
						PARALLEL^ { #PARALLEL->setType(PARALLEL_ACT); }
						parallelAct
					)?
				;
				
sequentialAct	:	primaryAct
					(
						SEQUENTIAL^ { #SEQUENTIAL->setType(SEQUENTIAL_ACT); }
						sequentialAct
					)?
				;
				
primaryAct		:	concreteAct
				|	LBRACE! parallelAct RBRACE!
				;
				
concreteAct		:	walkAct
				|	walkzAct
				|	levelAct
				|	winGameAct
				|	looseGameAct
				|	printAct
				|	waitAct
				|	qsendAct
				|	ssceneAct
				|	incAct
				|	decAct
				|	enableCounterAct
				|	disableCounterAct
				|	camfocusAct
				|	refOrErrorAct
				|	expAct
				|	moneyAct
				|	reportAct
				|	pulseAct
				|	giveAct
				|	addAct
				|	resetAct
				|	addRankAct
				|	addHeroAct
				|	removeHeroAct
				|	phaseAct
				|	forceAddHeroAct
				|	enableLevelAct
				|	disableLevelAct
				|	disappearAct
				|	camsetAct
				|	camsplineAct
				|	movieAct
				|	spawnAct
				|	destructAct
				|	shootAct
				|	killAct
				|	dieAct
				|	addRecordAct
				|	eraseRecordAct
				|	sayAct
				|	openAct
				|	closeAct
				|	addNewsAct
				|	camfollowAct
				|	alignmentAct
				;
				
alignmentAct	:	TKN_ALIGNMENT^ { #TKN_ALIGNMENT->setType(ALIGNMENT_ACT); }
					LBRACE! identifier COMMA! identifier COMMA! identifier RBRACE!
				;
				
refOrErrorAct	:	(identifier LBRACE) => errorAct
				|	refAct
				;
				
errorAct!		{
					int line = 0, column = 0;
					std::string unkact;
				}
				:	{
						line = LT(1)->getLine();
						column = LT(1)->getColumn();
						unkact = LT(1)->getText();
					}
					identifier LBRACE errorActParam (COMMA errorActParam)* RBRACE
					{
						ACT_LOG("WARNING: " << std::dec << getFilename() << "(" << line
								<< "): column " << column << ": unknown act element [" << unkact << "]\n");
					}
				;
				
errorActParam	:	identifier | floatNumber
				;

camfollowAct	:	TKN_CAMFOLLOW^ { #TKN_CAMFOLLOW->setType(CAMFOLLOW_ACT); }
					LBRACE! identifier RBRACE!
				;
				
addNewsAct		:	TKN_ADDNEWS^ { #TKN_ADDNEWS->setType(ADDNEWS_ACT); }
					LBRACE! identifier RBRACE!
				;
				
openAct			:	TKN_OPEN^ { #TKN_OPEN->setType(OPEN_ACT); }
					LBRACE! identifier RBRACE!
				;

closeAct		:	TKN_CLOSE^ { #TKN_CLOSE->setType(CLOSE_ACT); }
					LBRACE! identifier RBRACE!
				;
				
sayAct			:	TKN_SAY^ { #TKN_SAY->setType(SAY_ACT); }
					LBRACE! identifier
						(
							(	COMMA!
							|	a:DOT!
								{
									ACT_LOG("ERROR: " << std::dec << getFilename() << "(" << a->getLine()
											<< "): column " << a->getColumn() << ": unexpected \".\"\n");
								}
							)
							identifier
						)?
					RBRACE!
				;
				
addRecordAct	:	TKN_ADDRECORD^ { #TKN_ADDRECORD->setType(ADDRECORD_ACT); }
					LBRACE! identifier COMMA! identifier COMMA! identifier RBRACE!
				;
				
eraseRecordAct	:	TKN_ERASERECORD^ { #TKN_ERASERECORD->setType(ERASERECORD_ACT); }
					LBRACE! identifier RBRACE!
				;
				
shootAct		:	TKN_SHOOT^ { #TKN_SHOOT->setType(SHOOT_ACT); }
					LBRACE! identifier COMMA! identifier RBRACE!
				;
				
killAct			:	TKN_KILL^ { #TKN_KILL->setType(KILL_ACT); }
					LBRACE! identifier COMMA! identifier RBRACE!
				;

dieAct			:	TKN_DIE^ { #TKN_DIE->setType(DIE_ACT); }
					LBRACE! identifier RBRACE!
				;
				
destructAct		:	TKN_DESTRUCT^ { #TKN_DESTRUCT->setType(DESTRUCT_ACT); }
					LBRACE! identifier RBRACE!
				;
				
spawnAct		:	(	TKN_SPAWN^ { #TKN_SPAWN->setType(SPAWN_ACT); }
					|	TKN_SPAWNZ^ { #TKN_SPAWNZ->setType(SPAWN_ACT); }
					)
					LBRACE! identifier RBRACE!
				;
				
movieAct		:	TKN_MOVIE^ { #TKN_MOVIE->setType(MOVIE_ACT); }
					LBRACE! identifier (COMMA! identifier)? RBRACE!
				;
				
camsetAct		:	TKN_CAMSET^ { #TKN_CAMSET->setType(CAMSET_ACT); }
					LBRACE! identifier RBRACE!
				;
				
camsplineAct	:	(	TKN_CAMSPLINE^ { #TKN_CAMSPLINE->setType(CAMSPLINE_ACT); }
					|	TKN_CAM^ { #TKN_CAM->setType(CAMSPLINE_ACT); }
					)
					LBRACE! identifier (COMMA! floatNumber)? RBRACE!
				;
				
disappearAct	:	TKN_DISAPPEAR^ { #TKN_DISAPPEAR->setType(DISAPPEAR_ACT); }
					LBRACE! identifier RBRACE!
				;
				
phaseAct		:	TKN_PHASE^ { #TKN_PHASE->setType(PHASE_ACT); }
					LBRACE! integer RBRACE!
				;

enableLevelAct	:	TKN_ENABLELEVEL^ { #TKN_ENABLELEVEL->setType(ENABLELEVEL_ACT); }
					LBRACE! identifier RBRACE!
				;

disableLevelAct	:	TKN_DISABLELEVEL^ { #TKN_DISABLELEVEL->setType(DISABLELEVEL_ACT); }
					LBRACE! identifier RBRACE!
				;
				
addHeroAct		:	TKN_ADDHERO^ { #TKN_ADDHERO->setType(ADDHERO_ACT); }
					LBRACE! identifier RBRACE!
				;

forceAddHeroAct	:	TKN_FORCEADDHERO^ { #TKN_FORCEADDHERO->setType(FORCEADDHERO_ACT); }
					LBRACE! identifier RBRACE!
				;

removeHeroAct	:	TKN_REMOVEHERO^ { #TKN_REMOVEHERO->setType(REMOVEHERO_ACT); }
					LBRACE! identifier RBRACE!
				;

reportAct		:	TKN_REPORT^ { #TKN_REPORT->setType(REPORT_ACT); }
					LBRACE! parallelAct RBRACE!
				;
				
expAct			:	TKN_EXP^ { #TKN_EXP->setType(EXP_ACT); }
					LBRACE! integer RBRACE!
				;

moneyAct		:	TKN_MONEY^ { #TKN_MONEY->setType(MONEY_ACT); }
					LBRACE! integer RBRACE!
				;
				
refAct			:	identifier
					{
						#refAct = #([REF_ACT],#refAct);
					}
				;
				
camfocusAct		:	TKN_CAMFOCUS^ { #TKN_CAMFOCUS->setType(CAMFOCUS_ACT); }
					LBRACE! identifier RBRACE!
				;
				
walkAct			:	(	TKN_WALK^ { #TKN_WALK->setType(WALK_ACT); }
					|	TKN_RUN^ { #TKN_RUN->setType(RUN_ACT); }
					|	TKN_CRAWL^ { #TKN_CRAWL->setType(CRAWL_ACT); }
					)
					LBRACE!
						identifier COMMA!
						integer COMMA!
						integer
					RBRACE!
				;

walkzAct		:	(	TKN_WALKZ^ { #TKN_WALKZ->setType(WALKZ_ACT); }
					|	TKN_RUNZ^ { #TKN_RUNZ->setType(RUNZ_ACT); }
					|	TKN_CRAWLZ^ { #TKN_CRAWLZ->setType(CRAWLZ_ACT); }
					)
					LBRACE!
						identifier COMMA!
						identifier
					RBRACE!
				;
				
levelAct		:	TKN_LEVEL^ { #TKN_LEVEL->setType(LEVEL_ACT); }
					LBRACE! identifier RBRACE!
				;
				
winGameAct		:	TKN_WIN^ { #TKN_WIN->setType(WINGAME_ACT); }
				;

looseGameAct	:	TKN_LOOSE^ { #TKN_LOOSE->setType(LOOSEGAME_ACT); }
				;
				
printAct		:	TKN_PRINT^ { #TKN_PRINT->setType(PRINT_ACT); }
					LBRACE! (identifier|string) RBRACE!
				;
				
waitAct			:	TKN_WAIT^ { #TKN_WAIT->setType(WAIT_ACT); }
					LBRACE! floatNumber RBRACE!
				;
				
qsendAct		{
					int line = 0, column = 0;
					std::string toktext;
				}
				:	TKN_QSEND^ { #TKN_QSEND->setType(QSEND_ACT); }
					LBRACE!
						identifier COMMA! 
						(	questMessage
						|	{
								line = LT(1)->getLine();
								column = LT(1)->getColumn();
								toktext = LT(1)->getText();
							}
							identifier
							{
								ACT_LOG("ERROR: " << std::dec << getFilename() << "(" << line
										<< "): column " << column << ": unexpected identifier \""
										<< toktext << "\"\n");
								#qsendAct = antlr::nullAST;
							}
						)
					RBRACE!
				;

ssceneAct		:	TKN_SSCENE^ { #TKN_SSCENE->setType(SSCENE_ACT); }
					LBRACE! parallelAct RBRACE!
				;
				
pulseAct		:	TKN_PULSE^ { #TKN_PULSE->setType(PULSE_ACT); }
					LBRACE! identifier COMMA! integer RBRACE!
				;

giveAct			:	TKN_GIVE^ { #TKN_GIVE->setType(GIVE_ACT); }
					LBRACE! identifier (COMMA! identifier)? RBRACE!
				;

resetAct		:	TKN_RESET^ { #TKN_RESET->setType(RESET_ACT); }
					LBRACE! identifier RBRACE!
				;				

addAct			:	TKN_ADD^ { #TKN_ADD->setType(ADD_ACT); }
					LBRACE! identifier COMMA! integer RBRACE!
				;				

incAct			{
					int line = 0, column = 0;
					std::string toktext;
				}
				:	TKN_INC^ { #TKN_INC->setType(INC_ACT); }
					LBRACE!
						identifier
						(	{
								line = LT(1)->getLine();
								column = LT(1)->getColumn();
								toktext = LT(1)->getText();
							}
							errorInDecIncAct
							{
								ACT_LOG("ERROR: " << std::dec << getFilename() << "(" << line << "): column "
										<< column << ": unexpected \"" << toktext << "\" in inc act\n");
								#incAct = antlr::nullAST;
							}
						)?
					RBRACE!
				;				

errorInDecIncAct:	(COMMA! | DOT!) (identifier!|floatNumber!)
				;


decAct			{
					int line = 0, column = 0;
					std::string toktext;
				}
				:	TKN_DEC^ { #TKN_DEC->setType(DEC_ACT); }
					LBRACE!
						identifier
						(	{
								line = LT(1)->getLine();
								column = LT(1)->getColumn();
								toktext = LT(1)->getText();
							}
							errorInDecIncAct
							{
								ACT_LOG("ERROR: " << std::dec << getFilename() << "(" << line << "): column "
										<< column << ": unexpected \"" << toktext << "\" in dec act\n");
								#decAct = antlr::nullAST;
							}
						)?
					RBRACE!
				;
				
enableCounterAct:	TKN_ENABLECOUNTER^ { #TKN_ENABLECOUNTER->setType(ENABLECOUNTER_ACT); }
					LBRACE! identifier RBRACE!
				;		

disableCounterAct:	TKN_DISABLECOUNTER^ { #TKN_DISABLECOUNTER->setType(DISABLECOUNTER_ACT); }
					LBRACE! identifier RBRACE!
				;
				
addRankAct		:	TKN_ADDRANK^ { #TKN_ADDRANK->setType(ADDRANK_ACT); }
					LBRACE! identifier RBRACE!
				;
				
questMessage	:	TKN_QUESTTASKCOMPLETED
				|	TKN_QUESTTASKFAILED
				|	TKN_QUESTSTARTTASKCOMPLETED
				|	TKN_QUESTSTARTTASKFAILED
				|	TKN_QUESTENDTASKCOMPLETED
				|	TKN_QUESTENDTASKFAILED
				;
				
integer			:	INTEGER
				;
				
floatNumber		:	FLOATTYPE { #FLOATTYPE->setType(FLOATNODE); }
				|	INTEGER { #INTEGER->setType(FLOATNODE); }
				;
			
string			:	a:STRING
					{
						std::string temp = a->getText();
						std::string::size_type pos = 0;
						
						do
						{
							pos = temp.find('\\',pos);
							if(pos != std::string::npos)
							{
								if(pos != temp.size()-1)
								{
									switch(temp[pos+1])
									{
									case 'n':
										temp.replace(pos,2,"\n");	
										break;
									case 'r':
										temp.replace(pos,2,"\r");	
										break;
									case 't':
										temp.replace(pos,2,"\t");	
										break;
									case '\\':
										temp.replace(pos,2,"\\");	
										break;
									}
								}
								++pos;
							}
						}
						while(pos != std::string::npos);
						a->setText(temp);
					}
				;
			
identifier		:	IDENTIFIER
				;
				
//=====================================================================================//
//                         class ActScriptLexer extends Lexer;                         //
//=====================================================================================//
class ActScriptLexer extends Lexer;
options
{
	k=5;
	charVocabulary = '\3'..'\377';
}
tokens
{
	TKN_WALK="walk";				// +walk
	TKN_WALKZ="walkz";				// +walk into zone
	TKN_RUN="run";					// +run
	TKN_RUNZ="runz";				// +run into zone
	TKN_CRAWL="crawl";				// +crawl
	TKN_CRAWLZ="crawlz";			// +crawl into zone
	TKN_CAMFOLLOW="camfollow";		// +camera starts following person
	TKN_CAMFOCUS="camfocus";		// +camera focuses on person
	TKN_CAMSPLINE="camspline";		// +camera starts going on spline
	TKN_CAM="cam";					// +camera starts going on spline
	TKN_CAMSET="camset";			// +camera fixes at specified spline start point
	TKN_DIE="die";					// +specified enemy dies
	TKN_KILL="kill";				// +person1 shoots and kills person2
	TKN_SHOOT="shoot";				// +person1 shoots person2 and doesn't kill him
	TKN_LEVEL="level";				// +change level
	TKN_WIN="wingame";				// +player wins the game
	TKN_LOOSE="loosegame";			// +player looses the game
//	TKN_MESSAGE="msg";				// ???
	TKN_SAY="say";					// +somebody says a message to player
	TKN_PHASE="phase";				// +change phase to specified phase
	TKN_ENABLELEVEL="enable_level";	// +enables specified level exit
	TKN_DISABLELEVEL="disable_level";	// +disables specified level exit
	TKN_MOVIE="movie";				// +play bik movie
	TKN_SPAWN="spawn";				// +spawn person at specified point
	TKN_SPAWNZ="spawnz";			// +spawn person in specified zone
	TKN_PRINT="print";				// +print a message in a game console
	TKN_WAIT="wait";				// +just wait for some seconds
	TKN_OPEN="open";				// +open specified door
	TKN_CLOSE="close";				// +close specified door
	TKN_QSEND="qsend";				// +send message to quest
	TKN_GIVE="give";				// +give specified itemset to specified person
	TKN_EXP="exp";					// +give specified amount of exp
	TKN_MONEY="money";				// +give specified amount of money
	TKN_SSCENE="sscene";			// +script scene
	TKN_REPORT="report";			// +report reward given
	TKN_PULSE="pulse";				// +add val to counter then subtract val from counter
	TKN_ENABLECOUNTER="enable_counter";		// enables specified counter
	TKN_DISABLECOUNTER="disable_counter";	// disables specified counter
	TKN_ADDRANK="add_rank";			// +adds a rank to specified entity
	TKN_ADDHERO="add_hero";			// +adds a hero to a player team
	TKN_FORCEADDHERO="force_add_hero";
	TKN_REMOVEHERO="remove_hero";	// +removes a hero from a player team
	TKN_DISAPPEAR="disappear";		// +the hero disappears from a level
	TKN_DESTRUCT="destruct";		// +destroys specified object on level
	TKN_ADDRECORD="addrec";			// +adds a journal record
	TKN_ERASERECORD="eraserec";		// +erases a journal record
	TKN_ADDNEWS="add_news";			// +adds a news record
	TKN_ALIGNMENT="alignment";		// +changes alignment of specified force to specified state
	
	TKN_TASK="task";
	TKN_BRING="bring";				// bring specified itemset to specified person
	TKN_BRING2="bring2";			// bring specified itemset to specified person
	TKN_DEAD="dead";				// kill somebody, but killer team doesn't matter
//	TKN_KILL="kill";				// kill somebody, but killer must be from player team
	TKN_ALIVE="alive";				// somebody must be alive to complete
	TKN_STUB="stub";				// a stub task that never completes
	TKN_TALK="talk";				// talk with somebody
	TKN_TALK2="talk2";				// talk with somebody
	TKN_ELIMINATE="eliminate";		// eliminate force on specified level
	
	TKN_COMPLETED="completed";			// quest is completed
	TKN_NOTCOMPLETED="notcompleted";	// task is not completed
	TKN_FAILED="failed";				// quest is failed
	TKN_PASSED="passed";				// quest is passed either completed or failed
	TKN_STARTED="started";				// quest started
	TKN_NOTSTARTED="not_started";		// quest isn't started
	TKN_EPISODE="episode";				// quest is valid in specified episode only
	TKN_HAVEHERO="hero";				// quest starts only if team has specified hero
	TKN_NOHERO="nohero";				// quest starts only if team hasn't specified hero
	TKN_CONTEXT="context";				// quest starts only if context condition met (level and phase)
	//TKN_LEVEL="level";				// quest starts only if hero is on specified level

	TKN_ON="on";
	
	TKN_COUNTER="counter";			// signal declaration
	TKN_INC="inc";					// adds one to counter
	TKN_DEC="dec";					// subtracts one from counter
	TKN_ADD="add";					// adds a value to counter
	TKN_RESET="reset";				// resets state of counter
	
	TKN_ONKILL="on_kill";			// signals if specified entity killed by player
	TKN_ONDEAD="on_dead";			// signals if specified entity dies
	TKN_ONSTURN="on_start_turn";	// signals if turn is started
	TKN_ONETURN="on_end_turn";		// signals if turn is ended
	TKN_ONUSE="on_use";				// signals if player used specified object
	TKN_ONTALK="on_talk";			// signals if player talks with specified entity
	TKN_ONLEAVELEVEL="on_leave_level";	// signals if player leaves level

	TKN_DEPENDSON="depends_on";
	TKN_QUESTTASKCOMPLETED="quest_task_completed";
	TKN_QUESTTASKFAILED="quest_task_failed";
	TKN_QUESTSTARTTASKCOMPLETED="quest_start_task_completed";
	TKN_QUESTSTARTTASKFAILED="quest_start_task_failed";
	TKN_QUESTENDTASKCOMPLETED="quest_end_task_completed";
	TKN_QUESTENDTASKFAILED="quest_end_task_failed";
	
	TKN_TRIGGER="trigger";
	TKN_STRIGGER="strigger";
	TKN_CONDITION="condition";
	
	TKN_QUEST="quest";
//	TKN_TASK="task";
	TKN_STARTTASK="start_task";
	TKN_ENDTASK="end_task";
	TKN_DISABLED="disabled";
	TKN_QUESTHOLDER="holder";
	
	TKN_START="start";
	TKN_FAIL="failure";
	TKN_REMIND="remind";
	TKN_SUCCEED="success";
	TKN_COMPLONSTART="completed_on_start";
	TKN_COMPLONS="complons";
	TKN_FAILONSTART="failed_on_start";
	TKN_FAILONS="failons";
	TKN_PRE_SUCCEED="pre_success";
//	TKN_REMINDTASK="remind";
}

WHITESPACE			:	
					(
						options { generateAmbigWarnings=false; }
						:
						(	' '
						|	'\t'
						|	'\r' '\n'		{newline();}
						|	'\r'			{newline();}
						|	'\n'			{newline();}
						|	COMMENT
						) { $setType(antlr::Token::SKIP); }
					)
					;

protected
ML_COMMENT_DATA		:
					(
						options { generateAmbigWarnings=false; }
						:
							{LA(2)!='/'}? '*'
						|	'\r' '\n'		{newline();}
						|	'\r'			{newline();}
						|	'\n'			{newline();}
						|	~('*'|'\n'|'\r')
					)*
					;

protected
COMMENT				:	'/'
						(
							'/'
							{
								while(LA(1) != '\r' && LA(1) != '\n' && LA(1) != EOF)
								{
									consume();
								}
							}
							(	'\r' ('\n')?	{ newline(); }
							|	'\n'			{ newline(); }
							)? { $setType(antlr::Token::SKIP); }
						|	'*' ML_COMMENT_DATA "*/" { $setType(antlr::Token::SKIP); }
						)
					;

IDENTIFIER			:	(LETTER|UNDERSCORE) (LETTER|UNDERSCORE|DIGIT)*
						{
							std::string temp = getText();
							std::transform(temp.begin(),temp.end(),temp.begin(),tolower);
							setText(temp);
						}
					;

DOT					:	'.' ;
COMMA				:	',' ;
LCURLY				:	'{' ;
RCURLY				:	'}' ;

COLON				:	':' ;
SEMICOLON			:	';' ;

L_OR_LE				:	'<' { $setType(LESS); } ( '=' { $setType(LESSEQUAL); } )? ;
G_OR_GE				:	'>' { $setType(GREATER); } ( '=' { $setType(GREATEREQUAL); } )? ;
NOTEQUAL			:	"!=" ;

LBRACE				:	'(' ;
RBRACE				:	')' ;

PARALLEL			:	'|' ;
SEQUENTIAL			:	'&' ;

EQUALS				:	'=' ;

protected DIGIT			:	'0'..'9';
protected LETTER		:	(('a'..'z') | ('A'..'Z'));
protected UNDERSCORE	:	'_';
protected PLUS			:	'+';
protected MINUS			:	'-';

protected QUOTE			:	'"';

protected STRING_DATA	:
						(
							options { generateAmbigWarnings=false; }
							:
								'\\' ('"'|'n'|'r'|'t')
							|	~('"'|'\r'|'\n'|'\t')
						)*
						;

STRING					:	QUOTE
							a:STRING_DATA
							QUOTE
							{
								$setText(a->getText());
							}
						;

INT_OR_FLOAT			:	(PLUS|MINUS)?
							(DIGIT)+ { $setType(INTEGER); }
							(
								DOT (DIGIT)+
								(
									('e'|'E') ('+'|'-')? (DIGIT)+
								)?
								{ $setType(FLOATTYPE); }
							)?
						;
						
class ActTreeParser extends TreeParser;
{
private:
	std::string m_fileName;
	
public:
	void setFilename(const std::string &n) { m_fileName = n; }
	const std::string &getFilename() const { return m_fileName; }

private:
	struct CounterParams
	{
		PoL::HCondition m_inc, m_dec;
	};

	typedef PoL::ActTemplate::ActCreator_t AC_t;
	typedef PoL::ActTemplate::Iterator_t It_t;
	typedef PoL::Persistent::Id_t Id_t;

	static AC_t walk(const std::string &, int, int);
	static AC_t walkz(const std::string &, const std::string &);
	static AC_t run(const std::string &, int, int);
	static AC_t runz(const std::string &, const std::string &);
	static AC_t crawl(const std::string &, int, int);
	static AC_t crawlz(const std::string &, const std::string &);
	static AC_t level(const std::string &);
	static AC_t winGame();
	static AC_t looseGame();
	static AC_t wait(float);
	static AC_t print(const std::string &);
	static AC_t qsend(const std::string &, PoL::QuestMessage);
	static AC_t sscene();
	static AC_t report();
	static AC_t camfocus(const std::string &);
	static AC_t inc(const std::string &);
	static AC_t dec(const std::string &);
	static AC_t add(const std::string &, int);
	static AC_t reset(const std::string &);
	static void ref(PoL::HActTemplate, const std::string &, It_t);
	static AC_t pulse(const std::string &, int);
	static AC_t exp(int);
	static AC_t money(int);
	static AC_t give(const std::string &, const std::string &);
	static AC_t addRank(const std::string &);
	static AC_t addHero(const std::string &);
	static AC_t forceAddHero(const std::string &);
	static AC_t removeHero(const std::string &);
	static AC_t phase(int);
	static AC_t enableLevel(const std::string &);
	static AC_t disableLevel(const std::string &);
	static AC_t disappear(const std::string &);
	static AC_t movie(const std::string &, const std::string &);
	static AC_t spawn(const std::string &);
	static AC_t camspline(const std::string &, float);
	static AC_t destruct(const std::string &);
	static AC_t shoot(const std::string &, const std::string &);
	static AC_t kill(const std::string &, const std::string &);
	static AC_t die(const std::string &);
	static AC_t addrec(const std::string &, const std::string &, const std::string &);
	static AC_t eraserec(const std::string &);
	static AC_t say(const std::string &, const std::string &);
	static AC_t open(const std::string &);
	static AC_t close(const std::string &);
	static AC_t addnews(const std::string &);
	static AC_t camfollow(const std::string &);
	static AC_t alignment(const std::string &, const std::string &, const std::string &);
	static AC_t seq();
	static AC_t par();
	
	static void registerTemplate(const std::string &, PoL::HActTemplate);
	
	static PoL::HTask createTaskList(Id_t, const std::string &, const std::vector<PoL::HTask> &);
	static PoL::HTask createStubTask(Id_t, const std::string &);
	static PoL::HTask createKillTask(Id_t, const std::string &, const std::string &);
	static PoL::HTask createDeadTask(Id_t, const std::string &, const std::string &);
	static PoL::HTask createTalkTask(Id_t, const std::string &, const std::string &);
	static PoL::HTask createTalk2Task(Id_t, const std::string &, const std::string &);
	static PoL::HTask createBringTask(Id_t, const std::string &, const std::string &, const std::string &, PoL::HCondition);
	static PoL::HTask createBring2Task(Id_t, const std::string &, const std::string &, const std::string &, PoL::HCondition);
	static PoL::HTask createEliminateTask(Id_t, const std::string &, const std::string &, const std::string &);
	
	static PoL::HCondition getRefCondition(const std::string &);
	static PoL::HCondition createCompareCondition(Id_t, const std::string &, PoL::LogicalCondition, PoL::HCounter, PoL::HCounter);
	static PoL::HCondition createQuestCondition(Id_t, const std::string &, PoL::QuestConditionState, const std::string &);
	static PoL::HCondition createEpisodeCondition(Id_t, const std::string &, int);
	static PoL::HCondition createNoHeroCondition(Id_t, const std::string &, const std::string &);
	static PoL::HCondition createHaveHeroCondition(Id_t, const std::string &, const std::string &);
	static PoL::HCondition createOrCondition(Id_t, const std::string &, PoL::HCondition, PoL::HCondition);
	static PoL::HCondition createAndCondition(Id_t, const std::string &, PoL::HCondition, PoL::HCondition);
	static PoL::HCondition createContextCondition(Id_t, const std::string &, const std::string &, int);
	static PoL::HCondition createTaskCondition(Id_t, const std::string &, PoL::HTask, PoL::TaskState);
	static PoL::HCondition createAliveCondition(Id_t, const std::string &, const std::string &);
	static PoL::HCondition createDeadCondition(Id_t, const std::string &, const std::string &);
	
	static PoL::HTrigger createTrigger(Id_t, const std::string &, PoL::HCondition, PoL::HActTemplate, bool);
	
	static PoL::HCounter createCounter(Id_t, const std::string &, int, const CounterParams &);
	static PoL::HCounter createRefCounter(Id_t, const std::string &, const std::string &);
	static PoL::HCounter createConstCounter(Id_t, const std::string &, int);
	
	static PoL::HCondition createBringSignal(Id_t, const std::string &, const std::string &, const std::string &);
	static PoL::HCondition createKillSignal(Id_t, const std::string &, const std::string &);
	static PoL::HCondition createDeadSignal(Id_t, const std::string &, const std::string &);
	static PoL::HCondition createUseSignal(Id_t, const std::string &, const std::string &, const std::string &);
	static PoL::HCondition createTalkSignal(Id_t, const std::string &, const std::string &);
	static PoL::HCondition createStartTurnSignal(Id_t, const std::string &);
	static PoL::HCondition createEndTurnSignal(Id_t, const std::string &);
	static PoL::HCondition createLeaveLevelSignal(Id_t, const std::string &, const std::string &, const std::string &);
	
	static Id_t reserveQuestId(const std::string &);
	static Id_t reserveConditionId(const std::string &);
	static Id_t reserveTaskId(const std::string &);
	static Id_t reserveTriggerId(const std::string &);
	static Id_t reserveCounterId(const std::string &);
	
	static int makeInt(const antlr::RefAST &a) { return boost::lexical_cast<int>(a->getText()); }
	
	static PoL::HQuest makeQuest(const std::string &, Id_t, const PoL::QuestParams &);

	void reportError(const antlr::RecognitionException& ex);
	void reportError(const std::string& s);
	void reportWarning(const std::string& s);
}

program	:	(statement)*
		;
		
statement
		:	actDef
		|	taskDef
		|	triggerDef
		|	questDef
		|	conditionDef
		|	counterDef
		;
		
counterDef
		{
			bool disabled = false;
			std::string name;
			int ctr = 0;
			Id_t id;
			PoL::HCondition inc, dec;
			CounterParams params;
		}
		:	#(	
				COUNTER_DEFINE 
				(counterDisabled { disabled = true; } )?
				name=identifier
				{
					if(PoL::ActFactory::instance()->isCounterNameReserved(name))
					{
						ACT_LOG("WARNING: duplicate counter name found: [" << name << "]\n");
					}
					id = reserveCounterId(name);
				}
				
				(ctr=integer)?
				
				(	inc=counterIncParams[name]
				|	dec=counterDecParams[name]
				)*
				
				{
					params.m_inc = inc;
					params.m_dec = dec;
					createCounter(id,name,ctr,params);
				}
			)
		;
		
counterDisabled
		:	COUNTER_DISABLED
		;
		
		
counterIncParams[const std::string &name] returns [PoL::HCondition sig]
		{
			std::ostringstream sstr;
			sstr << name << ":inc";
			const std::string newname = sstr.str();
		}
		:	#(INC_COUNTER sig=condition[newname])
		;

counterDecParams[const std::string &name] returns [PoL::HCondition sig]
		{
			std::ostringstream sstr;
			sstr << name << ":dec";
			const std::string newname = sstr.str();
		}
		:	#(DEC_COUNTER sig=condition[newname])
		;

questDef
		{
			std::string name;
			bool disabled = false;
			PoL::QuestParams params;
		}
		:	#(QUEST_DEFINE name=identifier
				{
					if(PoL::ActFactory::instance()->isQuestNameReserved(name))
					{
						ACT_LOG("WARNING: duplicate quest name found: [" << name << "]\n");
					}
				}
				(questDisabled { disabled=true; })?
				params=questProperties[name]
			)
			{
				Id_t id = reserveQuestId(name);
				params.m_enabled = !disabled;
				makeQuest(name,id,params);
			}
		;
		
questDisabled
		:	QUEST_DISABLED
		;
		
questProperties[const std::string &name] returns [PoL::QuestParams params]
		{
			PoL::HTask task, start, end;
			PoL::HCondition remind, depends;
			PoL::HActTemplate onStart, onFail, onSuccess, onRemind, onComplons, onFailons, onPreSucceed;
			std::string holder;
		}
		:
		(	task=questTask[name]
		|	start=questStartTask[name]
		|	end=questEndTask[name]
		|	holder=questHolder[name]
		|	onStart=questStartAct[name]
		|	onFail=questFailAct[name]
		|	onSuccess=questSuccessAct[name]
		|	onRemind=questRemindAct[name]
		|	remind=questRemindCond[name]
		|	onComplons=questComplonsAct[name]
		|	onFailons=questFailonsAct[name]
		|	onPreSucceed=questOnPreSucceedAct[name]
		|	depends=questDependency[name]
		)*
		{
			params.m_dependency = depends;
			params.m_start = start;
			params.m_end = end;
			params.m_quest = task;
			params.m_holder = holder;
			params.m_remind = remind;
			params.m_onFail = onFail;
			params.m_onStart = onStart;
			params.m_onSucceed = onSuccess;
			params.m_onRemind = onRemind;
			params.m_onCompletedOnStart = onComplons;
			params.m_onFailedOnStart = onFailons;
			params.m_onPreSucceed = onPreSucceed;
		}
		;
		
questDependency[const std::string &name] returns [PoL::HCondition cond]
		{
			std::ostringstream sstr;
			sstr << name << ":depends";
			const std::string newname = sstr.str();
		}
		:	#(QUEST_DEPENDENCY cond=condition[newname])
		;
			
		
questRemindCond[const std::string &name] returns [PoL::HCondition cond]
		{
			std::ostringstream sstr;
			sstr << name << ":remind";
			const std::string newname = sstr.str();
		}
		:	#(QUEST_REMIND_TASK_DEFINE cond=condition[newname])
		;

questFailonsAct[const std::string &name] returns [PoL::HActTemplate action]
		:	#(QUEST_ON_FAILED_ON_START_ACT action=act)
		;

questOnPreSucceedAct[const std::string &name] returns [PoL::HActTemplate action]
		:	#(QUEST_ON_PRE_SUCCEED action=act)
		;

questComplonsAct[const std::string &name] returns [PoL::HActTemplate action]
		:	#(QUEST_ON_COMPLETED_ON_START_ACT action=act)
		;

questStartAct[const std::string &name] returns [PoL::HActTemplate action]
		:	#(QUEST_ON_START_ACT action=act)
		;

questFailAct[const std::string &name] returns [PoL::HActTemplate action]
		:	#(QUEST_ON_FAIL_ACT action=act)
		;

questSuccessAct[const std::string &name] returns [PoL::HActTemplate action]
		:	#(QUEST_ON_SUCCEED_ACT action=act)
		;

questRemindAct[const std::string &name] returns [PoL::HActTemplate action]
		:	#(QUEST_ON_REMIND_ACT action=act)
		;
		
questTask[const std::string &name] returns [PoL::HTask task]
		{
			std::ostringstream sstr;
			sstr << name << ":quest";
			const std::string newname = sstr.str();
		}
		:	#(QUEST_TASK_DEFINE task=taskBody[newname])
		;

questStartTask[const std::string &name] returns [PoL::HTask task]
		{
			std::ostringstream sstr;
			sstr << name << ":start";
			const std::string newname = sstr.str();
		}
		:	#(QUEST_START_TASK_DEFINE task=taskBody[newname])
		;
		
questEndTask[const std::string &name] returns [PoL::HTask task]
		{
			std::ostringstream sstr;
			sstr << name << ":end";
			const std::string newname = sstr.str();
		}
		:	#(QUEST_END_TASK_DEFINE task=taskBody[newname])
		;
		
questHolder[const std::string &name] returns [std::string holder]
		:	#(QUEST_HOLDER_DEFINE holder=identifier)
		;

triggerDef
		:	#(TRIGGER_DEFINE triggerDefImpl[false])
		|	#(STRIGGER_DEFINE triggerDefImpl[true])
		;
		
triggerDefImpl[bool onetime]
		{
			std::string name, condname;
			PoL::HCondition cond;
			PoL::HActTemplate action;
		}
		:	name=identifier
			{
				if(PoL::ActFactory::instance()->isTriggerNameReserved(name))
				{
					ACT_LOG("WARNING: duplicate trigger name found: [" << name << "]\n");
				}
				condname = name + ":condition";
			}
				
			cond=condition[condname]
			action=act
			{
				Id_t id = reserveTriggerId(name);
				createTrigger(id,name,cond,action,onetime);
			}
		;
		
conditionDef
		{
			std::string name;
			PoL::HCondition cond;
		}
		:	#(COND_DEFINE
				name=identifier
				{
					if(PoL::ActFactory::instance()->isConditionNameReserved(name))
					{
						ACT_LOG("WARNING: duplicate condition name found: [" << name << "]\n");
					}
				}
				cond=condition[name]
			)
		;
		
condition[const std::string &name] returns [PoL::HCondition cond]
		:	#(COND_BODY cond=conditionImpl[name])
		;
		
conditionImpl[const std::string &name] returns [PoL::HCondition cond]
		:	cond=concreteCondition[name]
		;
		
concreteCondition[const std::string &name] returns [PoL::HCondition cond]
		:	cond=orCondition[name]
		|	cond=andCondition[name]
		|	cond=completedCondition[name]
		|	cond=failedCondition[name]
		|	cond=startedCondition[name]
		|	cond=passedCondition[name]
		|	cond=notStartedCondition[name]
		|	cond=refCondition[name]
		|	cond=onKillSignal[name]
		|	cond=onDeadSignal[name]
		|	cond=onTalkSignal[name]
		|	cond=onUseSignal[name]
		|	cond=onEndTurnSignal[name]
		|	cond=onStartTurnSignal[name]
		|	cond=compareCondition[name]
		|	cond=haveHeroCondition[name]
		|	cond=noHeroCondition[name]
		|	cond=episodeCondition[name]
		|	cond=onLeaveLevelSignal[name]
		|	cond=contextCondition[name]
		|	cond=completedTaskCondition[name]
		|	cond=failedTaskCondition[name]
		|	cond=notCompletedTaskCondition[name]
		|	cond=aliveCondition[name]
		|	cond=deadCondition[name]
		;
		
deadCondition[const std::string &name] returns [PoL::HCondition cond]
		{
			std::string who;
		}
		:	#(DEAD_COND who=identifier)
			{
				Id_t id = reserveConditionId(name);
				cond = createDeadCondition(id,name,who);
			}
		;
		
aliveCondition[const std::string &name] returns [PoL::HCondition cond]
		{
			std::string who;
		}
		:	#(ALIVE_COND who=identifier)
			{
				Id_t id = reserveConditionId(name);
				cond = createAliveCondition(id,name,who);
			}
		;
		
completedTaskCondition[const std::string &name] returns [PoL::HCondition cond]
		{
			PoL::HTask task;
			const std::string taskname = name + ":task";
		}
		:	#(COMPLETED_TASK_COND task=taskBody[taskname])
			{
				Id_t id = reserveConditionId(name);
				cond = createTaskCondition(id,name,task,PoL::tsCompleted);
			}
		;

failedTaskCondition[const std::string &name] returns [PoL::HCondition cond]
		{
			PoL::HTask task;
			const std::string taskname = name + ":task";
		}
		:	#(FAILED_TASK_COND task=taskBody[taskname])
			{
				Id_t id = reserveConditionId(name);
				cond = createTaskCondition(id,name,task,PoL::tsFailed);
			}
		;

notCompletedTaskCondition[const std::string &name] returns [PoL::HCondition cond]
		{
			PoL::HTask task;
			const std::string taskname = name + ":task";
		}
		:	#(NOTCOMPLETED_TASK_COND task=taskBody[taskname])
			{
				Id_t id = reserveConditionId(name);
				cond = createTaskCondition(id,name,task,PoL::tsNotCompleted);
			}
		;
		
contextCondition[const std::string &name] returns [PoL::HCondition cond]
		{
			std::string lev;
			int phase = -1;
		}
		:	#(CONTEXT_COND lev=identifier (phase=integer)?)
			{
				Id_t id = reserveConditionId(name);
				cond = createContextCondition(id,name,lev,phase);
			}
		;

onLeaveLevelSignal[const std::string &name] returns [PoL::HCondition cond]
		{
			std::string lev,to;
		}
		:	#(ONLEAVELEVEL_SIGNAL lev=identifier (to=identifier)? )
			{
				Id_t id = reserveConditionId(name);
				cond = createLeaveLevelSignal(id,name,lev,to);
			}
		;

haveHeroCondition[const std::string &name] returns [PoL::HCondition cond]
		{
			std::string hero;
		}
		:	#(HAVEHERO_COND hero=identifier)
			{
				Id_t id = reserveConditionId(name);
				cond = createHaveHeroCondition(id,name,hero);
			}
		;

noHeroCondition[const std::string &name] returns [PoL::HCondition cond]
		{
			std::string hero;
		}
		:	#(NOHERO_COND hero=identifier)
			{
				Id_t id = reserveConditionId(name);
				cond = createNoHeroCondition(id,name,hero);
			}
		;
		
episodeCondition[const std::string &name] returns [PoL::HCondition cond]
		{
			int episode;
		}
		:	#(EPISODE_COND episode=integer)
			{
				Id_t id = reserveConditionId(name);
				cond = createEpisodeCondition(id,name,episode);
			}
		;

counterArg[const std::string &name] returns [PoL::HCounter ctr]
		{
			int val = 0;
		}
		:	val=integer
			{
				Id_t id = reserveCounterId(name);
				ctr = createConstCounter(id,name,val);
			}
		|	ctr=refCounter[name]
		;
		
refCounter[const std::string &name] returns [PoL::HCounter ctr]
		{
			std::string ctrname;
		}
		:	#(REF_COUNTER ctrname=identifier)
			{
				Id_t id = reserveCounterId(name);
				ctr = createRefCounter(id,name,ctrname);
			}
		;
		
compareCondition[const std::string &name] returns [PoL::HCondition cond]
		{
			PoL::HCounter l, r;
			PoL::LogicalCondition lc;
			const std::string lname = name+":left";
			const std::string rname = name+":right";
		}
		:	(	#(LESS_COND			l=counterArg[lname] r=counterArg[rname] { lc = PoL::lcLess;         } )
			|	#(GREATER_COND		l=counterArg[lname] r=counterArg[rname] { lc = PoL::lcGreater;      } )
			|	#(GREATEREQUAL_COND	l=counterArg[lname] r=counterArg[rname] { lc = PoL::lcGreaterEqual; } )
			|	#(LESSEQUAL_COND	l=counterArg[lname] r=counterArg[rname] { lc = PoL::lcLessEqual;    } )
			|	#(EQUAL_COND		l=counterArg[lname] r=counterArg[rname] { lc = PoL::lcEqual;        } )
			|	#(NOTEQUAL_COND		l=counterArg[lname] r=counterArg[rname] { lc = PoL::lcNotEqual;     } )
			)
			
			{
				Id_t id = reserveConditionId(name);
				cond = createCompareCondition(id,name,lc,l,r);
			}
		;

onKillSignal[const std::string &name] returns [PoL::HCondition cond]
		{
			std::string subj;
		}
		:	#(ONKILL_SIGNAL subj=identifier)
			{
				Id_t id = reserveConditionId(name);
				cond = createKillSignal(id,name,subj);
			}
		;

onDeadSignal[const std::string &name] returns [PoL::HCondition cond]
		{
			std::string subj;
		}
		:	#(ONDEAD_SIGNAL subj=identifier)
			{
				Id_t id = reserveConditionId(name);
				cond = createDeadSignal(id,name,subj);
			}
		;
		
onTalkSignal[const std::string &name] returns [PoL::HCondition cond]
		{
			std::string subj;
		}
		:	#(ONTALK_SIGNAL subj=identifier)
			{
				Id_t id = reserveConditionId(name);
				cond = createTalkSignal(id,name,subj);
			}
		;
		
onUseSignal[const std::string &name] returns [PoL::HCondition cond]
		{
			std::string obj;
			std::string subj;
		}
		:	#(ONUSE_SIGNAL subj=identifier (obj=identifier)?)
			{
				Id_t id = reserveConditionId(name);
				cond = createUseSignal(id,name,obj,subj);
			}
		;

onStartTurnSignal[const std::string &name] returns [PoL::HCondition cond]
		:	ONSTURN_SIGNAL
			{
				Id_t id = reserveConditionId(name);
				cond = createStartTurnSignal(id,name);
			}
		;

onEndTurnSignal[const std::string &name] returns [PoL::HCondition cond]
		:	ONETURN_SIGNAL
			{
				Id_t id = reserveConditionId(name);
				cond = createEndTurnSignal(id,name);
			}
		;
		
orCondition[const std::string &name] returns [PoL::HCondition cond]
		{
			PoL::HCondition cond1, cond2;
			const std::string lname = name+":left";
			const std::string rname = name+":right";
		}
		:	#(OR_COND cond1=concreteCondition[lname] cond2=concreteCondition[rname])
			{
				Id_t id = reserveConditionId(name);
				cond = createOrCondition(id,name,cond1,cond2);
			}
		;

andCondition[const std::string &name] returns [PoL::HCondition cond]
		{
			PoL::HCondition cond1, cond2;
			const std::string lname = name+":left";
			const std::string rname = name+":right";
		}
		:	#(AND_COND cond1=concreteCondition[lname] cond2=concreteCondition[rname])
			{
				Id_t id = reserveConditionId(name);
				cond = createAndCondition(id,name,cond1,cond2);
			}
		;

completedCondition[const std::string &name] returns [PoL::HCondition cond]
		{
			std::string questname;
		}
		:	#(COMPLETED_QUEST_COND questname=identifier)
			{
				Id_t id = reserveConditionId(name);
				cond = createQuestCondition(id,name,PoL::qcsCompleted,questname);
			}
		;

failedCondition[const std::string &name] returns [PoL::HCondition cond]
		{
			std::string questname;
		}
		:	#(FAILED_QUEST_COND questname=identifier)
			{
				Id_t id = reserveConditionId(name);
				cond = createQuestCondition(id,name,PoL::qcsFailed,questname);
			}
		;


passedCondition[const std::string &name] returns [PoL::HCondition cond]
		{
			std::string questname;
		}
		:	#(PASSED_QUEST_COND questname=identifier)
			{
				Id_t id = reserveConditionId(name);
				cond = createQuestCondition(id,name,PoL::qcsPassed,questname);
			}
		;

startedCondition[const std::string &name] returns [PoL::HCondition cond]
		{
			std::string questname;
		}
		:	#(STARTED_QUEST_COND questname=identifier)
			{
				Id_t id = reserveConditionId(name);
				cond = createQuestCondition(id,name,PoL::qcsStarted,questname);
			}
		;

notStartedCondition[const std::string &name] returns [PoL::HCondition cond]
		{
			std::string questname;
		}
		:	#(NOTSTARTED_QUEST_COND questname=identifier)
			{
				Id_t id = reserveConditionId(name);
				cond = createQuestCondition(id,name,PoL::qcsNotStarted,questname);
			}
		;

refCondition[const std::string &name] returns [PoL::HCondition cond]
		{
			std::string refname;
		}
		:	#(REF_COND refname=identifier)
			{
				cond = getRefCondition(refname);
			}
		;
		
taskDef	{
			std::string name;
			PoL::HTask task;
		}
		:	#(TASK_DEFINE name=identifier
				{
					if(PoL::ActFactory::instance()->isTaskNameReserved(name))
					{
						ACT_LOG("WARNING: duplicate task name found: [" << name << "]\n");
					}
				}
				
				task=taskBody[name])
		;
		
taskBody[const std::string &name] returns [PoL::HTask task]
		:	#(TASK_BODY task=taskImpl[name])
		;
		
taskImpl[const std::string &name] returns [PoL::HTask result]
		{
			PoL::HTask task;
			std::vector<PoL::HTask> tasks;
			unsigned int counter = 0;
		}
		:	(	{
					std::ostringstream sstr;
					sstr << name << ":" << counter++;
					const std::string newname = sstr.str();
				}
				task=concreteTask[newname]
				{
					tasks.push_back(task);
				}
			)*
			{
				Id_t id = reserveTaskId(name);
				result = createTaskList(id,name,tasks);
			}
		;
		
concreteTask[const std::string &name] returns [PoL::HTask task]
		:	task=bringTask[name]
		|	task=bring2Task[name]
		|	task=killTask[name]
		|	task=deadTask[name]
		|	task=stubTask[name]
		|	task=talkTask[name]
		|	task=talk2Task[name]
		|	task=eliminateTask[name]
		;
		
eliminateTask[const std::string &name] returns [PoL::HTask task]
		{
			std::string force, level;
		}
		:	#(ELIMINATE_TASK force=identifier level=identifier)
			{
				Id_t id = reserveTaskId(name);
				task = createEliminateTask(id,name,force,level);
			}
		;

bringTask[const std::string &name] returns [PoL::HTask task]
		{
			std::string items, pers;
		}
		:	#(BRING_TASK items=identifier pers=identifier)
			{
				std::string signame = name+":signal";
				
				Id_t ids = reserveConditionId(signame);
				PoL::HCondition cond = createBringSignal(ids,signame,pers,items);
				
				Id_t id = reserveTaskId(name);
				task = createBringTask(id,name,pers,items,cond);
			}
		;

bring2Task[const std::string &name] returns [PoL::HTask task]
		{
			std::string items, pers;
		}
		:	#(BRING2_TASK items=identifier pers=identifier)
			{
				std::string signame = name+":signal";
				
				Id_t ids = reserveConditionId(signame);
				PoL::HCondition cond = createBringSignal(ids,signame,pers,items);
				
				Id_t id = reserveTaskId(name);
				task = createBring2Task(id,name,pers,items,cond);
			}
		;
		
killTask[const std::string &name] returns [PoL::HTask task]
		{
			std::string victim;
		}
		:	#(KILL_TASK victim=identifier)
			{
				Id_t id = reserveTaskId(name);
				task = createKillTask(id,name,victim);
			}
		;

talkTask[const std::string &name] returns [PoL::HTask task]
		{
			std::string subj;
		}
		:	#(TALK_TASK subj=identifier)
			{
				Id_t id = reserveTaskId(name);
				task = createTalkTask(id,name,subj);
			}
		;

talk2Task[const std::string &name] returns [PoL::HTask task]
		{
			std::string subj;
		}
		:	#(TALK2_TASK subj=identifier)
			{
				Id_t id = reserveTaskId(name);
				task = createTalk2Task(id,name,subj);
			}
		;
		
deadTask[const std::string &name] returns [PoL::HTask task]
		{
			std::string victim;
		}
		:	#(DEAD_TASK victim=identifier)
			{
				Id_t id = reserveTaskId(name);
				task = createDeadTask(id,name,victim);
			}
		;
		
stubTask[const std::string &name] returns [PoL::HTask task]
		{
			Id_t id = reserveTaskId(name);
		}
		:	STUB_TASK
			{
				task = createStubTask(id,name);
			}
		;
		
actDef	{
			std::string name;
			PoL::HActTemplate a;
		}
		:	#(ACT_DEFINE name=identifier a=act { registerTemplate(name,a); } )
		;

act returns[PoL::HActTemplate result]
		{
			result.reset(new PoL::ActTemplate);
			It_t it = result->end();
		}
		:	#(ACT_BODY actImpl[result,it])
		;
		
actImpl[PoL::HActTemplate at, It_t it]
		:	parAct[at,it]
		|	seqAct[at,it]
		|	walkAct[at,it]
		|	walkzAct[at,it]
		|	levelAct[at,it]
		|	winGameAct[at,it]
		|	looseGameAct[at,it]
		|	printAct[at,it]
		|	waitAct[at,it]
		|	qsendAct[at,it]
		|	ssceneAct[at,it]
		|	incAct[at,it]
		|	decAct[at,it]
		|	enableCounterAct[at,it]
		|	camfocusAct[at,it]
		|	refAct[at,it]
		|	expAct[at,it]
		|	moneyAct[at,it]
		|	reportAct[at,it]
		|	pulseAct[at,it]
		|	giveAct[at,it]
		|	addAct[at,it]
		|	resetAct[at,it]
		|	addRankAct[at,it]
		|	addHeroAct[at,it]
		|	removeHeroAct[at,it]
		|	forceAddHeroAct[at,it]
		|	phaseAct[at,it]
		|	enableLevelAct[at,it]
		|	disableLevelAct[at,it]
		|	disappearAct[at,it]
		|	movieAct[at,it]
		|	camsplineAct[at,it]
		|	spawnAct[at,it]
		|	destructAct[at,it]
		|	shootAct[at,it]
		|	killAct[at,it]
		|	dieAct[at,it]
		|	addRecordAct[at,it]
		|	eraseRecordAct[at,it]
		|	sayAct[at,it]
		|	openAct[at,it]
		|	closeAct[at,it]
		|	addNewsAct[at,it]
		|	camfollowAct[at,it]
		|	alignmentAct[at,it]
		;
		
alignmentAct[PoL::HActTemplate at, It_t it]
		{
			std::string team1, team2, al;
		}
		:	#(ALIGNMENT_ACT team1=identifier team2=identifier al=identifier)
			{
				at->addActCreator(alignment(team1,team2,al),it);
			}
		;

camfollowAct[PoL::HActTemplate at, It_t it]
		{
			std::string pers;
		}
		:	#(CAMFOLLOW_ACT pers=identifier)
			{
				at->addActCreator(camfollow(pers),it);
			}
		;

addNewsAct[PoL::HActTemplate at, It_t it]
		{
			std::string newsid;
		}
		:	#(ADDNEWS_ACT newsid=identifier)
			{
				at->addActCreator(addnews(newsid),it);
			}
		;
		
openAct[PoL::HActTemplate at, It_t it]
		{
			std::string door;
		}
		:	#(OPEN_ACT door=identifier)
			{
				at->addActCreator(open(door),it);
			}
		;

closeAct[PoL::HActTemplate at, It_t it]
		{
			std::string door;
		}
		:	#(CLOSE_ACT door=identifier)
			{
				at->addActCreator(close(door),it);
			}
		;
		
sayAct[PoL::HActTemplate at, It_t it]
		{
			std::string actor;
			std::string strid;
		}
		:	#(SAY_ACT actor=identifier (strid=identifier)?)
			{
				if(strid.empty()) actor.swap(strid);
				at->addActCreator(say(actor,strid),it);
			}
		;
		
addRecordAct[PoL::HActTemplate at, It_t it]
		{
			std::string jid, hdrid, msgid;
		}
		:	#(ADDRECORD_ACT jid=identifier hdrid=identifier msgid=identifier)
			{
				at->addActCreator(addrec(jid,hdrid,msgid),it);
			}
		;

eraseRecordAct[PoL::HActTemplate at, It_t it]
		{
			std::string jid;
		}
		:	#(ERASERECORD_ACT jid=identifier)
			{
				at->addActCreator(eraserec(jid),it);
			}
		;
		
shootAct[PoL::HActTemplate at, It_t it]
		{
			std::string shooter,target;
		}
		:	#(SHOOT_ACT shooter=identifier target=identifier)
			{
				at->addActCreator(shoot(shooter,target),it);
			}
		;

killAct[PoL::HActTemplate at, It_t it]
		{
			std::string shooter,target;
		}
		:	#(KILL_ACT shooter=identifier target=identifier)
			{
				at->addActCreator(kill(shooter,target),it);
			}
		;
		
dieAct[PoL::HActTemplate at, It_t it]
		{
			std::string target;
		}
		:	#(DIE_ACT target=identifier)
			{
				at->addActCreator(die(target),it);
			}
		;
		
destructAct[PoL::HActTemplate at, It_t it]
		{
			std::string obj;
		}
		:	#(DESTRUCT_ACT obj=identifier)
			{
				at->addActCreator(destruct(obj),it);
			}
		;

camsplineAct[PoL::HActTemplate at, It_t it]
		{
			std::string spl;
			float dur = 0.0f;
		}
		:	#(CAMSPLINE_ACT spl=identifier (dur=floatNumber)? )
			{
				at->addActCreator(camspline(spl,dur),it);
			}
		;
		
spawnAct[PoL::HActTemplate at, It_t it]
		{
			std::string who;
		}
		:	#(SPAWN_ACT who=identifier)
			{
				at->addActCreator(spawn(who),it);
			}
		;
		
movieAct[PoL::HActTemplate at, It_t it]
		{
			std::string bik,levl;
		}
		:	#(MOVIE_ACT bik=identifier (levl=identifier)?)
			{
				at->addActCreator(movie(bik,levl),it);
			}
		;
		
disappearAct[PoL::HActTemplate at, It_t it]
		{
			std::string subj;
		}
		:	#(DISAPPEAR_ACT subj=identifier)
			{
				at->addActCreator(disappear(subj),it);
			}
		;

phaseAct[PoL::HActTemplate at, It_t it]
		{
			int p = 0;
		}
		:	#(PHASE_ACT p=integer)
			{
				at->addActCreator(phase(p),it);
			}
		;
		
enableLevelAct[PoL::HActTemplate at, It_t it]
		{
			std::string lev;
		}
		:	#(ENABLELEVEL_ACT lev=identifier)
			{
				at->addActCreator(enableLevel(lev),it);
			}
		;

disableLevelAct[PoL::HActTemplate at, It_t it]
		{
			std::string lev;
		}
		:	#(DISABLELEVEL_ACT lev=identifier)
			{
				at->addActCreator(disableLevel(lev),it);
			}
		;

addHeroAct[PoL::HActTemplate at, It_t it]
		{
			std::string hero;
		}
		:	#(ADDHERO_ACT hero=identifier)
			{
				at->addActCreator(addHero(hero),it);
			}
		;

forceAddHeroAct[PoL::HActTemplate at, It_t it]
		{
			std::string hero;
		}
		:	#(FORCEADDHERO_ACT hero=identifier)
			{
				at->addActCreator(forceAddHero(hero),it);
			}
		;

removeHeroAct[PoL::HActTemplate at, It_t it]
		{
			std::string hero;
		}
		:	#(REMOVEHERO_ACT hero=identifier)
			{
				at->addActCreator(removeHero(hero),it);
			}
		;

pulseAct[PoL::HActTemplate at, It_t it]
		{
			std::string what;
			int val;
		}
		:	#(PULSE_ACT what=identifier val=integer)
			{
				at->addActCreator(pulse(what,val),it);
			}
		;
		
expAct[PoL::HActTemplate at, It_t it]
		{
			int expVal;
		}
		:	#(EXP_ACT expVal=integer)
			{
				at->addActCreator(exp(expVal),it);
			}
		;

moneyAct[PoL::HActTemplate at, It_t it]
		{
			int expVal;
		}
		:	#(MONEY_ACT expVal=integer)
			{
				at->addActCreator(money(expVal),it);
			}
		;

giveAct[PoL::HActTemplate at, It_t it]
		{
			std::string itset;
			std::string person;
		}
		:	#(GIVE_ACT itset=identifier (person=identifier)? )
			{
				at->addActCreator(give(itset,person),it);
			}
		;

refAct[PoL::HActTemplate at, It_t it]
		{
			std::string subj;
		}
		:	#(REF_ACT subj=identifier)
			{
				ref(at,subj,it);
				//at->addActCreator(ref(subj),it);
			}
		;

camfocusAct[PoL::HActTemplate at, It_t it]
		{
			std::string subj;
		}
		:	#(CAMFOCUS_ACT subj=identifier)
			{
				at->addActCreator(camfocus(subj),it);
			}
		;

resetAct[PoL::HActTemplate at, It_t it]
		{
			std::string ctr;
		}
		:	#(RESET_ACT ctr=identifier)
			{
				at->addActCreator(reset(ctr),it);
			}
		;

addAct[PoL::HActTemplate at, It_t it]
		{
			std::string ctr;
			int val;
		}
		:	#(ADD_ACT ctr=identifier val=integer)
			{
				at->addActCreator(add(ctr,val),it);
			}
		;

incAct[PoL::HActTemplate at, It_t it]
		{
			std::string ctr;
		}
		:	#(INC_ACT ctr=identifier)
			{
				at->addActCreator(inc(ctr),it);
			}
		;

decAct[PoL::HActTemplate at, It_t it]
		{
			std::string ctr;
		}
		:	#(DEC_ACT ctr=identifier)
			{
				at->addActCreator(dec(ctr),it);
			}
		;
		
enableCounterAct[PoL::HActTemplate at, It_t it]
		{
			std::string ctr;
		}
		:	#(ENABLECOUNTER_ACT ctr=identifier)
		;
		
addRankAct[PoL::HActTemplate at, It_t it]
		{
			std::string pers;
		}
		:	#(ADDRANK_ACT pers=identifier)
			{
				at->addActCreator(addRank(pers),it);
			}
		;

parAct[PoL::HActTemplate at, It_t it]
		{
			It_t itn;
		}
		:	#(
				PARALLEL_ACT 
				{
					itn = at->addActCreator(par(),it);
				}
				actImpl[at,itn]
				actImpl[at,itn]
			)
		;
		
seqAct[PoL::HActTemplate at, It_t it]
		{
			It_t itn;
		}
		:	#(
				SEQUENTIAL_ACT 
				{
					itn = at->addActCreator(seq(),it);
				}
				actImpl[at,itn]
				actImpl[at,itn]
			)
		;

walkAct[PoL::HActTemplate at, It_t it]
		{
			std::string name;
			int x, y;
		}
		:	#(WALK_ACT name=identifier x=integer y=integer)
			{
				at->addActCreator(walk(name,x,y),it);
			}
		|	#(RUN_ACT name=identifier x=integer y=integer)
			{
				at->addActCreator(run(name,x,y),it);
			}
		|	#(CRAWL_ACT name=identifier x=integer y=integer)
			{
				at->addActCreator(crawl(name,x,y),it);
			}
		;
		
walkzAct[PoL::HActTemplate at, It_t it]
		{
			std::string name;
			std::string zone;
		}
		:	#(WALKZ_ACT name=identifier zone=identifier)
			{
				at->addActCreator(walkz(name,zone),it);
			}
		|	#(RUNZ_ACT name=identifier zone=identifier)
			{
				at->addActCreator(runz(name,zone),it);
			}
		|	#(CRAWLZ_ACT name=identifier zone=identifier)
			{
				at->addActCreator(crawlz(name,zone),it);
			}
		;
		
levelAct[PoL::HActTemplate at, It_t it]
		{
			std::string lev;
		}
		:	#(LEVEL_ACT lev=identifier)
			{
				at->addActCreator(level(lev),it);
			}
		;
		
winGameAct[PoL::HActTemplate at, It_t it]
		:	WINGAME_ACT { at->addActCreator(winGame(),it); }
		;

looseGameAct[PoL::HActTemplate at, It_t it]
		:	LOOSEGAME_ACT { at->addActCreator(looseGame(),it); }
		;
		
printAct[PoL::HActTemplate at, It_t it]
		{
			std::string strid;
		}
		:	#(PRINT_ACT
				(	strid=identifier { strid = '$' + strid; }
				|	strid=string
				)
			)
			{
				at->addActCreator(print(strid),it);
			}
		;
		
waitAct[PoL::HActTemplate at, It_t it]
		{
			float tau;
		}
		:	#(WAIT_ACT tau=floatNumber)
			{
				at->addActCreator(wait(tau),it);
			}
		;
		
qsendAct[PoL::HActTemplate at, It_t it]
		{
			std::string quest;
			PoL::QuestMessage msg;
		}
		:	#(QSEND_ACT quest=identifier msg=questMessage)
			{
				at->addActCreator(qsend(quest,msg),it);
			}
		;

reportAct[PoL::HActTemplate at, It_t it]
		{
			It_t itn;
		}
		:	#(REPORT_ACT { itn = at->addActCreator(report(),it); } actImpl[at,itn])
		;
		
ssceneAct[PoL::HActTemplate at, It_t it]
		{
			It_t itn;
		}
		:	#(SSCENE_ACT { itn = at->addActCreator(sscene(),it); } actImpl[at,itn])
		;
		
questMessage returns [PoL::QuestMessage msg]
		:	TKN_QUESTTASKCOMPLETED			{ msg = PoL::qmTaskCompleted; }
		|	TKN_QUESTTASKFAILED				{ msg = PoL::qmTaskFailed; }
		|	TKN_QUESTSTARTTASKCOMPLETED		{ msg = PoL::qmStartTaskCompleted; }
		|	TKN_QUESTSTARTTASKFAILED		{ msg = PoL::qmStartTaskFailed; }
		|	TKN_QUESTENDTASKCOMPLETED		{ msg = PoL::qmEndTaskCompleted; }
		|	TKN_QUESTENDTASKFAILED			{ msg = PoL::qmEndTaskFailed; }
		;
		
string returns[std::string id]
		:	a:STRING { id = a->getText(); }
		;
		
identifier returns[std::string id]
		:	a:IDENTIFIER { id = a->getText(); }
		;
		
floatNumber returns[float n]
		:	a:FLOATNODE { n = boost::lexical_cast<float>(a->getText()); }
		;

integer returns[int n]
		:	a:INTEGER { n = boost::lexical_cast<int>(a->getText()); }
		;
