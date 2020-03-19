#
# $Id: CustomBuilder.py 599 2004-10-04 10:50:52Z gvozdoder $

def __customBuildGenerator(source,target,env,for_signature):
	pass

def createBuilder(env):
	builder = env.Builder(generator = __customBuildGenerator)
	env.Append(BUILDERS = {'CustomBuild' : builder})