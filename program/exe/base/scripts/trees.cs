#
#
#

# Make sure the server manager exists
#
if focusServer
	focusClient
	simTreeCreate ServerTree MainWindow server
	simTreeAddSet ServerTree manager
endif

# Client manager always exists
#
simTreeCreate ClientTree MainWindow
simTreeAddSet ClientTree manager
