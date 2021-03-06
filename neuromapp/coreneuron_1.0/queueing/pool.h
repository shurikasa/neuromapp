/*
 * Neuromapp - pool.h, Copyright (c), 2015,
 * Kai Langen - Swiss Federal Institute of technology in Lausanne,
 * kai.langen@epfl.ch,
 * All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3.0 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 */

/**
 * @file neuromapp/coreneuron_1.0/queueing/pool.h
 * \brief Contains Pool class declaration.
 */

#include <boost/array.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <time.h>
#include <ctime>
#include <numeric>

#include "coreneuron_1.0/queueing/thread.h"
#include "utils/storage/neuromapp_data.h"

#ifdef _OPENMP
#include <omp.h>
#endif

#ifndef MAPP_POOL_H_
#define MAPP_POOL_H_

namespace queueing {

template<implementation I>
class NrnThreadData;

template<implementation I>
class Pool {
private:
	int time_;
	bool v_;
	bool perform_algebra_;
	int percent_ITE_;
	int events_per_step_;
	int all_spiked_;
	const static int min_delay_ = 5;
	int percent_spike_;

	boost::array<NrnThreadData<I>,64> threadDatas;

public:
	/** \fn Pool(bool verbose, int eventsPer, int percent_ITE_, bool isSpike, bool algebra)
	    \brief initializes a Pool with a threadDatas array
	    \param verbose verbose mode: 1 = on, 0 = off
	    \param events_per_step_ number of events per time step
	    \param percent_ITE_ is the percentage of inter-thread events
	    \param isSpike determines whether or not there are spike events
	    \param algebra determines whether to perform linear algebra calculations
	 */
	explicit Pool(bool verbose=false, int eventsPer=0, int pITE=0, bool isSpike=0, bool algebra=0):
	v_(verbose), events_per_step_(eventsPer), percent_ITE_(pITE),
	perform_algebra_(algebra), all_spiked_(0), time_(0){
				percent_spike_ = isSpike ? 3:0;
				std::cout<<"isSpike = "<<isSpike<<std::endl;
   				srand(time(NULL));
	}

	/** \fn accumulate_stats()
	    \brief accumulates statistics from the threadData array and stores them using impl::storage
	 */
	void accumulate_stats(){
		int all_ite_received = 0;
		int all_enqueued = 0;
		int all_delivered = 0;
    	for(int i=0; i < threadDatas.size(); ++i){
			all_ite_received += threadDatas[i].ite_received_;
			all_enqueued += threadDatas[i].enqueued_;
			all_delivered += threadDatas[i].delivered_;
			if(v_){
				std::cout<<"Cellgroup "<<i<<" ite received: "
				<<threadDatas[i].ite_received_<<std::endl;
				std::cout<<"Cellgroup "<<i<<" enqueued: "<<
				threadDatas[i].enqueued_<<std::endl;
				std::cout<<"Cellgroup "<<i<<" delivered: "<<
				threadDatas[i].delivered_<<std::endl;
			}
		}

		if(v_){
			std::cout<<"Total inter-thread received: "<<all_ite_received<<std::endl;
			std::cout<<"Total enqueued: "<<all_enqueued<<std::endl;
			std::cout<<"Total spiked: "<<all_spiked_<<std::endl;
			std::cout<<"Total delivered: "<<all_delivered<<std::endl;
		}
		neuromapp_data.put_copy("inter_received", all_ite_received);
		neuromapp_data.put_copy("enqueued", all_enqueued);
	    neuromapp_data.put_copy("spikes", all_spiked_);
	    neuromapp_data.put_copy("delivered", all_delivered);
	}

	/** \fn void timeStep(int totalTime)
	    \brief master function to call generate, enqueue, and deliver
	    \param totalTime tells the provides the total simulation time
	 */
	void timeStep(int totalTime){
	    int size = threadDatas.size();
	    #pragma omp parallel for schedule(static,1)
	    for(int i=0; i < size; ++i){
			generateEvents(totalTime,i);

			threadDatas[i].enqueueMyEvents();
			//Have threads enqueue their interThreadEvents
			while(threadDatas[i].deliver(i, time_)); // deliver
			
			if(perform_algebra_)
				threadDatas[i].l_algebra();

	    }
	    time_++;
	}

	/** \fn void generateEvents(int totalTime, int myID)
	    \brief creates events which are sent to random destination threads
	    \param totalTime tells the provides the total simulation time
	    \param i the thread index
	 */
	void generateEvents(int totalTime, int myID){
	    //events can be generated with time range: (current time) to (current time + 10%)
	    int diff(1);
	    if(totalTime > 10)
	        diff = rand() % (totalTime/10);
	    /// Simulated target of a NetCon and the event time
	    double tt = double();
	    double data = double();
	    int dst_nt = myID;
	    for(int j=0; j < events_per_step_; ++j){
			//set time_ to be some time in the future t + diff
			tt = static_cast<double>(time_ + diff);
			if(percent_ITE_ > 0)
			    dst_nt = chooseDst(myID);
			else
			    dst_nt = myID;

			data = static_cast<double>(dst_nt);

			if (dst_nt == myID)
			    threadDatas[dst_nt].selfSend(data, tt);
			else
			    threadDatas[dst_nt].interThreadSend(data, tt + min_delay_);
	    }
	}

	/** \fn void handleSpike(int totalTime)
	    \brief compensates for the spike exchange by adding events every 5 timesteps
	    \param totalTime tells the provides the total simulation time
	 */
	void handleSpike(int totalTime){
   	    int diff(1);
   	    if(totalTime > 10)
			diff = rand() % (totalTime/10);

	    if( (time_ % min_delay_) == 0){
			//serial distribution of spike events to inter-thread events
			int num_spikes = min_delay_*threadDatas.size()*events_per_step_*percent_spike_/100;
			int dst(0);
			double data, tt;
			for(int i = 0; i < num_spikes; ++i){
			   tt = (double)(time_ + diff + min_delay_);
			   dst = rand() % threadDatas.size();
			   data = (double)dst;
			   threadDatas[dst].selfSend(data, tt);
			   all_spiked_++;
			}
	    }
	}

	/** \fn int chooseDst(int myID)
	    \brief Generates a random destination according to the variable percent_ITE_
	    \param myID the thread index
	    \return destination
	 */
	int chooseDst(int myID){
	    int dst = myID;
	    if ((rand() % 100) < percent_ITE_) //if destination is another thread
		while(dst == myID)
		    dst = rand() % threadDatas.size();

	    return dst;
	}
};

}
#endif
