#pragma once

//#include "../ProjectConfig/config/mlconfig.h"


#include <cassert>

#include <vector>
#include <hash_set>
#include <algorithm>

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include <mutual/object.h>
#include <mutual/handle.h>

#include <mll/debug/log.h>
#include <mll/utils/lcrn_tree.h>

#include <common/utils/vfilewpr.h>

#include <logic2/logicdefs.h>
#include <logic2/entityaux.h>
#include <logic2/activity.h>
#include <logic2/entity.h>
#include <logic2/hexgrid.h>
#include <logic2/hexutils.h>
#include <logic2/pathutils.h>
#include <logic2/graphic.h>
#include <logic2/spawn.h>
#include <logic2/dirtylinks.h>
#include <logic2/form.h>

#ifdef _HOME_VERSION
#	define ACT_LOG(L)		MLL_MAKE_LOG("act.log",L)
#else
#	define ACT_LOG(L)
#endif

#include "Utils.h"
#include "TasksForward.h"
#include "QuestsForward.h"
#include "SignalsForward.h"
#include "TriggersForward.h"
#include "CountersForward.h"
#include "FlexibleSSForward.h"
#include "ConditionsForward.h"