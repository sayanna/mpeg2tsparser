#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "ts_analyzer.hpp"

int main(int argc, char **argv)
{
	char c;
	ts_analyzer ts_test;

	while ((c = getopt(argc,argv,"amp:i:")) != -1) {
        switch (c)
        {
            case 'i':
            {
	            ts_test.set_file(optarg);
	            break;
            }

            case 'm':
            {
                ts_test.option = ts_analyzer::process_manual;
                break;
            }

            case 'a':
            {
            	ts_test.option = ts_analyzer::process_all;
            	break;
            }

            case 'p':
            {
            	int pid = atoi(optarg);
            	ts_test.add_search_pid(pid);
            	break;
            }

            default:
            break;
        }
 	}

 	ts_test.init();
 	ts_test.process();
 	ts_test.deinit();

 	return 0;
}
