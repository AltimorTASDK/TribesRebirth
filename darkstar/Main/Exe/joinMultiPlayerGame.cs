if test "$Server::Address" != ""
   if test "$Name" != ""
      connect $Server::Address
      tsView
   else
      echo "You must fill in the name field before you connect."
   endif
else
   echo "You must select a server before you connect."
endif