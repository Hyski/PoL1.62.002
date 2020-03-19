import re
import os
import string
import shutil
import os.path
import tkMessageBox

def checkFiles(home,files):
	for f in files:
		if not os.path.exists(home + '/' + f):
			raise 'File not found: ' + home + '/' + f
			
def extractRevision():
	#urlExtractor = re.compile('^URL: (.*?)$', re.MULTILINE)
	#revExtractor = re.compile('^r([0-9]+)', re.MULTILINE)
	lcrevExtractor = re.compile('^Last Changed Rev: ([0-9]+)$', re.MULTILINE)
	lcrevstr = lcrevExtractor.search(os.popen('svn info src').read())
	
	return lcrevstr.group(1)

def extractVersion():
	versionExtractor = re.compile('PoL_ver_major = (\d+),.*PoL_ver_minor = (\d+),.*PoL_ver_build = (\d+),', re.DOTALL)
	versstrs = versionExtractor.search(file('src/pol/undercover/version.cpp').read())
	version = (int(versstrs.group(1)),int(versstrs.group(2)),int(versstrs.group(3)))
	version_str = '%d.%02d.%03d' % version
	return version_str

def prepareDir(outdir):
	if not os.path.exists(outdir): os.mkdir(outdir)

def copyFiles(home,files,outdir):
	for f in files:
		print ' '*4 + home + '/' + f
		shutil.copyfile(home + '/' + f, outdir + '\\' + f)

def issueSvnCommand(command, *args):
	cmd = 'svn ' + command + ' --non-interactive ' + string.join(args,' ')
	os.system(cmd)

def makeTag(version):
	commonPrefix = 'http://hive/code/pol_missions/game/'
	issueSvnCommand('commit', '-m', '"autocommit for version ' + version + '"')
	issueSvnCommand('copy', commonPrefix + 'trunk', commonPrefix + 'tags/' + version, '-m', '"version ' + version + ' auto tag"')

def makeUpdate(home,files,postfix,tag = False):
	print 'Checking files...'
	checkFiles(home,files)
	print 'Extracting version...'
	version = extractVersion()
	revision = extractRevision()
	outdir = r'\\mighty\projects\pol_missions\bin' + '\\' + version + '.r' + revision + postfix
	print 'Creating', outdir, 'directory...'
	prepareDir(outdir)
	print 'Copying files...'
	copyFiles(home,files,outdir)
	if tag:
		print 'Making auto tag...'
		makeTag(version)
	print 'Done!'
	tkMessageBox.showinfo('PoL Missions', 'Well done!')	
	
if __name__ == '__main__':
	print extractRevision()