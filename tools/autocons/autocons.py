#
# $Id: autocons.py 686 2004-10-10 00:25:25Z gvozdoder $

import re
import copy
import string
import os.path
from types import *

from Builder import *
from Solution import *

g_defaultLibConf = dict(
	lua					= 'dll',
	vis					= 'dll',
	zlib				= 'dll',
	antlr				= 'dll',
	mll_fs				= 'dll',
	mutual				= 'dll',
	storage				= 'dll',
	stlport				= 'dll',
	cppunit				= 'dll',
	shaderfx			= 'dll',
	mll_core			= 'dll',
	vis_mesh			= 'dll',
	vis_buffer			= 'dll',
	boost_regex			= 'dll',
	boost_signals		= 'dll',
	vis_mesh_render		= 'dll',
	)

g_libToMacroTable = dict(
	lua					= ['lib', 'LUA_STATIC'],
	vis					= ['lib', 'VIS_BASIC_STATIC'],
	zlib				= ['dll', 'ZLIB_DLL'],
	antlr				= ['lib', 'ANTLR_STATIC'],
	mll_fs				= ['dll', 'FS_USE_DLL'],
	mutual				= ['lib', 'MUTUAL_STATIC'],
	storage				= ['lib', 'STORAGE_STATIC'],
	stlport				= ['lib', '_STLP_USE_STATIC_LIB'],
	cppunit				= ['dll', 'CPPUNIT_DLL'],
	shaderfx			= ['lib', 'SHADERFX_STATIC'],
	mll_core			= ['dll', 'MLL_USE_DLL'],
	vis_mesh			= ['lib', 'VIS_MESH_STATIC'],
	vis_buffer			= ['lib', 'VIS_BUFFER_STATIC'],
	boost_regex			= ['dll', 'BOOST_REGEX_DYN_LINK'],
	boost_signals		= ['dll', 'BOOST_SIGNALS_DYN_LINK'],
	vis_mesh_render		= ['lib', 'VIS_MESH_RENDER_STATIC'],
	)

g_configurations = dict(
	debug = dict(
		crt="dll",						# Determines crt type (dll, mt or st)
		debug=True,						# Disables optimizations
		libconf=g_defaultLibConf,
		disableLogs=False,				# Disables all logs
		homeVersion=True,				# Defines MLS_HOME_VERSION macro
		globalDefinesInclude=[],		# Global macro definitions
		globalDefinesExclude=[],		# Global macro definitions exclude
	),
    release = dict(
		crt="dll",						# Determines crt type (dll, mt or st)
		debug=False,					# Disables optimizations
		libconf=g_defaultLibConf,
		disableLogs=False,				# Disables all logs
		homeVersion=True,				# Defines MLS_HOME_VERSION macro
		globalDefinesInclude=[],		# Global macro definitions
		globalDefinesExclude=[],		# Global macro definitions exclude
	),
    outrelease = dict(
		crt="dll",						# Determines crt type (dll, mt or st)
		debug=False,					# Disables optimizations
		libconf=g_defaultLibConf,
		disableLogs=True,				# Disables all logs
		homeVersion=False,				# Defines MLS_HOME_VERSION macro
		globalDefinesInclude=[],		# Global macro definitions
		globalDefinesExclude=[],		# Global macro definitions exclude
	)
)

def build(slnname,env,cfgs):
	fullcfgs = prepareConfigs(cfgs)
	cenv = env.Copy(CC = os.path.normcase('tools/ctools/cl.exe'),
					RC = os.path.normcase('tools/ctools/rc.exe'),
					AR = os.path.normcase('tools/ctools/lib.exe'),
					LINK = os.path.normcase('tools/ctools/link.exe'),
					tools = ['msvc','mslib','mslink'])
	cenv.SConsignFile(slnname + '.dbm')
	Builder(Solution(slnname,fullcfgs),cenv).buildSolution(fullcfgs)

# Merge dictionaries
def mergeDict(udict, gdict):
	result = gdict
	
	for k, v in udict.iteritems():
		if type(v) is DictType and k in gdict and type(gdict[k]) is DictType:
			result[k] = mergeDict(v,gdict[k])
		else:
			result[k] = v;

	return result

# Merge user-defined properties with defaults
def mergeConfigs(configs):
	result = copy.deepcopy(g_configurations)
	
	if configs == None: return result

	# Clean configurations unused in user configs
	for cfgname in g_configurations.iterkeys():
		if cfgname not in configs: result.pop(cfgname)

	# Merge with user configs
	for cfgname,cfg in configs.iteritems():
		source = None
		# If cfgname absent in default configurations and cfg has 'copyFrom' member
		#   then use default configurations cfg['copyFrom'] as basis for current configuration
		if cfgname not in g_configurations and 'copyFrom' in cfg:
			source = copy.deepcopy(g_configurations[cfg['copyFrom']])
		else:
			source = copy.deepcopy(g_configurations[cfgname])

		result[cfgname] = mergeDict(cfg,source)

	return result

def prepareConfigs(configs):
	fullconfigs = mergeConfigs(configs)
	
	for cfgname in fullconfigs.iterkeys():
		cfg = fullconfigs[cfgname]
		
		for libname, libstatus in cfg['libconf'].iteritems():
			if libstatus == g_libToMacroTable[libname][0]:
				cfg['globalDefinesInclude'].append(g_libToMacroTable[libname][1])
			else:
				cfg['globalDefinesExclude'].append(g_libToMacroTable[libname][1])

	return fullconfigs


