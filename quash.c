char *dirToEnter;
			
			if(numOfArgs == 0) {
				dirToEnter = getenv("HOME");
				chdir (dirToEnter);
			}
			else if(numOfArgs == 1) {
				dirToEnter = args[1];
				chdir (dirToEnter);	
			}
