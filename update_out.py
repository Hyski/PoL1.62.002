import updater

files = ['muffle.dll', 'mutual.dll', 'polmis.exe',
		 'muffle.pdb', 'mutual.pdb', 'polmis.pdb']

updater.makeUpdate('executable/bin/outrelease', files, '_out')


