/*
    Based on original assignment by: Dr. R. Bettati, PhD
    Department of Computer Science
    Texas A&M University
    Date  : 2013/01/31
 */


#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <sstream>
#include <iomanip>

#include <sys/time.h>
#include <cassert>
#include <assert.h>

#include <cmath>
#include <numeric>
#include <algorithm>
#include <signal.h>

#include <list>
#include <vector>

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

#include "reqchannel.h"
#include "SafeBuffer.h"
#include "Histogram.h"
using namespace std;

struct req_args{
    // RequestChannel* channel;
    SafeBuffer* request_buffer;
    // Histogram* hist;
    int reqNum;
    string name;
};

struct worker_args{
    RequestChannel* channel;
    SafeBuffer* request_buffer;
    Histogram* hist;
    // int reqNum;
};

void err_handler(int arg){
    // system("rm fifo");
    // printf("Client Sigsev\n");
    // exit(0);
}

void* request_thread_function(void* arg) {
	/*
		Fill in this function.

		The loop body should require only a single line of code.
		The loop conditions should be somewhat intuitive.

		In both thread functions, the arg parameter
		will be used to pass parameters to the function.
		One of the parameters for the request thread
		function MUST be the name of the "patient" for whom
		the data requests are being pushed: you MAY NOT
		create 3 copies of this function, one for each "patient".
	 */
    struct req_args *req;
    req = (struct req_args*)arg;
	for(int i = 0; i < req->reqNum; i++) {
        //enqueue requests
        req -> request_buffer -> push(req->name);
	}
}

void* worker_thread_function(void* arg) {
    /*
		Fill in this function. 

		Make sure it terminates only when, and not before,
		all the requests have been processed.

		Each thread must have its own dedicated
		RequestChannel. Make sure that if you
		construct a RequestChannel (or any object)
		using "new" that you "delete" it properly,
		and that you send a "quit" request for every
		RequestChannel you construct regardless of
		whether you used "new" for it.
     */
    struct worker_args *req;
    req = (struct worker_args*)arg;
    while(true) {
        string request = req->request_buffer->pop();
			req->channel->cwrite(request);
            cout << "\nRequest: "<<request<<endl;
			if(request == "quit") {
			   	delete req->channel;
                break;
            }else{
				string response = req->channel->cread();
				req->hist->update (request, response);
                cout<<"\nUpdate histogram HERE!\n";
			}
    }
}

/*--------------------------------------------------------------------------*/
/* MAIN FUNCTION */
/*--------------------------------------------------------------------------*/

int main(int argc, char * argv[]) {
    // struct sigaction a;
    // sigemptyset(&a.sa_mask);
    // a.sa_handler = err_handler;
    // a.sa_flags=0;
    // sigaction(SIGSEGV,&a,0);

    int n = 100; //default number of requests per "patient"
    int w = 1; //default number of worker threads
    int opt = 0;
    while ((opt = getopt(argc, argv, "n:w:")) != -1) {
        switch (opt) {
            case 'n':
                n = atoi(optarg);
                break;
            case 'w':
                w = atoi(optarg); //This won't do a whole lot until you fill in the worker thread function
                break;
			}
    }

    int pid = fork();
	if (pid == 0){
		execl("dataserver", (char*) NULL);
	}
	else {
        cout << "n == " << n << endl;
        cout << "w == " << w << endl;

        cout << "CLIENT STARTED:" << endl;
        cout << "Establishing control channel... " << flush;
        RequestChannel *chan = new RequestChannel("control", RequestChannel::CLIENT_SIDE);
        cout << "done." << endl<< flush;

		SafeBuffer request_buffer;
		Histogram hist;

        struct req_args person[3];
        pthread_t req_threads[3];
        person[0].name="data John Smith";
        person[1].name="data Jane Smith";
        person[2].name="data Joe Smith";
        // string reqJane="data Jane Smith";
        // string reqJoe="data Joe Smith";
        for(int i = 0; i < 3; ++i) {
            person[i].request_buffer= &request_buffer;
            person[i].reqNum=n;
            pthread_create(&req_threads[i], NULL,request_thread_function,(void*)&person[i]);
            // pthread_create(&Jane, NULL,request_thread_function,&reqJane);

            // request_buffer.push("data John Smith");
        }
        cout << "Done populating request buffer" << endl;

    //join your req threads
    for(int i = 0; i< 3 ; i++){pthread_join(req_threads[i],NULL);}

        cout << "Pushing quit requests... ";
        for(int i = 0; i < w; ++i) {
            request_buffer.push("quit");
        }
        cout << "done." << endl;

        // chan->cwrite("newchannel");
		// string s = chan->cread ();
        // RequestChannel *workerChannel = new RequestChannel(s, RequestChannel::CLIENT_SIDE);

    timeval start,end;
    gettimeofday(&start,NULL);

    struct worker_args workers[w];
    //  req_threads[3];
    vector <pthread_t> threads;
	for(int i=0; i<=w;i++){
        chan->cwrite("newchannel");
		string s = chan->cread ();
        RequestChannel *workerChannel = new RequestChannel(s, RequestChannel::CLIENT_SIDE);

        //spawn worker thread

        workers[i].request_buffer = &request_buffer;
        workers[i].channel = workerChannel;
        workers[i].hist = &hist;
        

        pthread_create(&threads[i], NULL, worker_thread_function, (void*)&workers[i]);
        threads.push_back(threads[i]);

    }
	for(int i=1; i<w;i++){
        pthread_join(threads[i],NULL);
    }
        // while(true) {
        //     string request = request_buffer.pop();
		// 	workerChannel->cwrite(request);

		// 	if(request == "quit") {
		// 	   	delete workerChannel;
        //         break;
        //     }else{
		// 		string response = workerChannel->cread();
		// 		hist.update (request, response);
		// 	}
        // }
        chan->cwrite ("quit");
        delete chan;
        cout << "All Done!!!" << endl; 
		hist.print ();

    gettimeofday(&end,NULL);
    cout<< "\nTotal run time: "<<abs(end.tv_sec-start.tv_sec)<<"."<<abs(end.tv_usec-start.tv_usec)<<" sec\n";

    }
}
