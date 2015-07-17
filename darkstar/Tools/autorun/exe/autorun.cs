//--------------------------------------
function mainWindow()
{
   newTitleWindow("Starsiege", mainWin, 0,0, 500, 319);
   setIcon( mainWin, "starsiege.ico" );
   centerWindow(mainWin);
   
   newBitmap(mainWin, bitmap, "starsiege.bmp", 0,0);
   centerWindow(bitmap, true, true);
}


//--------------------------------------
function menuRun()
{
   mainWindow();
   newButton(mainWin, "Run", btn_run, 312, 110, 155, 20);
   newButton(mainWin, "www.starsiege.com", btn_web);
   newButton(mainWin, "Quit", btn_quit);

   showWindow( mainWin, true );
}


//--------------------------------------
function menuInstall()
{
   mainWindow();
   newButton(mainWin, "Install", btn_install, 312, 120, 155, 20);
   newButton(mainWin, "View Read Me", btn_readme);
   newButton(mainWin, "www.starsiege.com", btn_web);
   $last::y += 15;
   newButton(mainWin, "Quit", btn_quit);

   showWindow( mainWin, true );
}



//--------------------------------------
function btn_install::onPress()
{
   open("setup.exe");
   quit();
}

//--------------------------------------
function btn_run::onPress()
{
   open($app@"\\starsiege.exe");
   quit();
}

//--------------------------------------
function btn_web::onPress()
{
   open("StarsiegeWebsite.url");
}

//--------------------------------------
function btn_readme::onPress()
{
   open("readme.txt");
}

//--------------------------------------
function btn_quit::onPress()
{
   quit();
}

//--------------------------------------
function btn_explore::onPress()
{
   explore(".");
}


//--------------------------------------
function display()
{
   // is it currently running?
   if (isRunning("STARSIEGE_LOCK"))
      quit();
   else
   {
      //--------------------------------------
      // is it installed?
      if (registryOpen("/HKEY_LOCAL_MACHINE/Software/Dynamix/Starsiege"))
      {
         $app = registryRead("path");
         if ($app != "FALSE")
            menuRun();
         else
            menuInstall();

         registryClose();
      }
      else
         menuInstall();
   }
}


display();