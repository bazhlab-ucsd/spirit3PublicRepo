ClassesPath = ./classes/
SpiritSource = $(ClassesPath)coord.cpp $(ClassesPath)critter.cpp $(ClassesPath)timeStructure.cpp $(ClassesPath)trace.cpp $(ClassesPath)connection.cpp $(ClassesPath)cell.cpp $(ClassesPath)network.cpp $(ClassesPath)environment.cpp spirit.cpp $(ClassesPath)outputLogService.cpp $(ClassesPath)coord.h $(ClassesPath)critter.h $(ClassesPath)timeStructure.h $(ClassesPath)trace.h $(ClassesPath)connection.h $(ClassesPath)cell.h $(ClassesPath)network.h $(ClassesPath)environment.h $(ClassesPath)outputLogService.h $(ClassesPath)toStr.h $(ClassesPath)stageManager.h $(ClassesPath)stageManager.cpp
ConnectivityFolder = networkConnectivity
SpiritExecutableFileName = spirit143

all: init build

buildVisual: $(SpiritSource) sdl.cpp
	g++ -Wall $(SpiritSource) -O3 -g -lSDL -o $(SpiritExecutableFileName)

build: $(SpiritSource) sdl.cpp
	g++ -Wall $(SpiritSource) -O3 -g -D__no_sdl__ -o $(SpiritExecutableFileName)

run: $(SpiritExecutableFileName)
	./$(SpiritExecutableFileName) \
	-output_location out \
	-mode_init 0 \
	-h_init 1 \
	-v_init 0 \
	-dpos_init -1 \
	-dneg_init 0 \
	-mode_swt 0 \
	-h_swt 0 \
	-v_swt 1 \
	-dpos_swt 0 \
	-dneg_swt -1 \
	-train_time 0 \
	-switch_time1 0 \
	-switch_time2 0 \
	-DeserializeCellStateFile DState \
	-DeserializeWeightFile DWeight \
	-suffix 0 
	#./$(SpiritExecutableFileName) -output_location out -mode_init 1 -h_init 1 -v_init -1 -dpos_init 0 -dneg_init 0 -mode_swt 1 -h_swt -1 -v_swt 1 -dpos_swt 0 -dneg_swt 0 -train_time 0 -switch_time1 0 -switch_time2 0 -suffix 0
	#./$(SpiritExecutableFileName) -output_location out -hidden_neuron_dropoff_seed 10 -init_network StartWeights -mode_init 1 -h_init 1 -v_init -1 -dpos_init 0 -dneg_init 0 -mode_swt 1 -h_swt -1 -v_swt 1 -dpos_swt 0 -dneg_swt 0 -train_time 0 -switch_time1 0 -switch_time2 0 -suffix 0

clean:
	-rm create_prelearned class enviro.txt output.conn* create_env create_network happiness.* output/output.conn.* output/happiness.* STDIN* output/conn_sd.out* output/output_sum.*

distclean:
	-rm twol.conn flat.conn rand10.env

init:
	@[ -e rand10.env ] || make rand10.env
	@[ -e twol.conn ] || make twol.conn

flat.conn:
	g++ create_network.cpp -o create_network
	./create_network

prelearned.conn:
	g++ create_prelearned_conn.cpp -o create_prelearned
	./create_prelearned

twol.conn:sin_network_parametrized.cpp
	#g++ twol_network.cpp -o twol_network
	#g++ sin_network.cpp -o sin_network
	#g++ sin_network_random.cpp -o sin_network_random
	#g++ sin_reduced.cpp -o sin_reduced
	g++ sin_network_parametrized.cpp -o sin_network_parametrized
	#./twol_network
	#./sin_network
	#./sin_reduced
	#./sin_network_random
	./sin_network_parametrized
	#no_in2mid_inhib
	cat twol.conn |grep -v '^0.*-' > .tt
	mv .tt twol.conn

networkSweep:sin_network_parametrized.cpp $(ClassesPath)outputLogService.h $(ClassesPath)outputLogService.cpp
	g++ sin_network_parametrized.cpp $(ClassesPath)outputLogService.h $(ClassesPath)outputLogService.cpp -o sin_network_parametrized
	./sin_network_parametrized -r_direction_i 1 -r_direction_j 1 -network 1.conn
	./sin_network_parametrized -r_direction_i 2 -r_direction_j 1 -network 2.conn
	./sin_network_parametrized -r_direction_i 3 -r_direction_j 1 -network 3.conn
	./sin_network_parametrized -r_direction_i 2 -r_direction_j 2 -network 4.conn
	./sin_network_parametrized -r_direction_i 3 -r_direction_j 2 -network 6.conn
	./sin_network_parametrized -r_direction_i 3 -r_direction_j 3 -network 9.conn
	
	mkdir -p $(ConnectivityFolder) 
	for i in 1 2 3 4 6 9; do( \
	cat $${i}.conn | grep -v '^0.*-' > $(ConnectivityFolder)/$${i}.conn; \
	rm $${i}.conn; \
	); done

rand10.env:
	g++ create_env.cpp -o create_env
	./create_env

collection:
	for i in `seq 0 9` ; do (./$(SpiritExecutableFileName) -output_location out_swt_p_sleep -mode_init 0 -h_init 1 -v_init -1 -dpos_init 0 -dneg_init 0 -mode_swt 0 -h_swt 0 -v_swt 0 -dpos_swt 1 -dneg_swt -1 -train_time 0 -switch_time1 2000 -switch_time2 10000 -sleepy_time 4000 -suffix $$i &) ; done

collection_additive:
	D=`pwd`; i=0; for e in `seq 0.05 0.005 0.5`; do echo "cd $$D; ./class -gui 0 -excitability $$e -suffix $${i}  -aeons 50000 " ; i=`echo "$$i+1"|bc`;  done

stdp_test:
	g++ -Wall classes.cpp -O2 -g -o class -D__no_sdl__
	D=`pwd`; s=0.0001; for i in `seq 0 1 119`; do echo "cd $$D; ./class -gui 0 -stdp_scale $$s -suffix $${i}  -aeons 15000 " |qsub; s=`echo "scale=6; $$s*1.1"|bc`; done

sweep_strength:
	g++ -Wall classes.cpp -O2 -g -o class -D__no_sdl__
	D=`pwd`; s=0.002; for i in `seq 0 1 15`; do echo "cd $$D; ./class -gui 0 -strength_bound $$s -suffix $${i}  -aeons 20000 "; s=`echo "scale=6; $$s*1.58"|bc`; done

biocluster:
	g++ -Wall classes.cpp -O2 -g -o class -D__no_sdl__
	D=`pwd`; for i in `seq 0 1 30`; do echo "cd $$D; ./class -gui 0 -suffix $${i} -aeons 50000"|qsub ; done

sweep_in2mid:
	g++ sin_network_parametrized.cpp -o sin_network_parametrized
	g++ -Wall classes.cpp -O2 -g -o class -D__no_sdl__
	D=`pwd`; for i in `seq 1 1 10`; do echo "cd $$D; ./sin_network_parametrized -middle_fan_in $${i} -network twol.conn.$${i} ; time ./class -init_network twol.conn.$${i} -gui 0 -suffix $${i}  -aeons 3 "|qsub;  done
	D=`pwd`; for i in `seq 15 5 45` 49; do echo "cd $$D; ./sin_network_parametrized -middle_fan_in $${i} -network twol.conn.$${i} ; time ./class -init_network twol.conn.$${i} -gui 0 -suffix $${i}  -aeons 3 "|qsub; done

sweep:
	for sw_t in 500 2000 5000; do	for i in `seq 1 8`; do ( ./$(SpiritExecutableFileName) -output_location out_swt_p -mode_init 1 -h_init 1 -v_init -1 -dpos_init 0 -dneg_init 0 -mode_swt 1 -h_swt 0 -v_swt 0 -dpos_swt 1 -dneg_swt -1 -train_time 0 -suffix $$((i+sw_t)) -switch_time1 $${sw_t} -switch_time2 $$((sw_t+5000))&  ) ; done ;	done
	
sweep_rhn:
	for i in 1 2 3 4 5 6 7 8; do( \
	./$(SpiritExecutableFileName) \
	-output_location out$${i} \
	-hidden_neuron_dropoff_seed $${i}  \
	-init_network StartWeights  \
	-mode_init 1 \
	-h_init 1 \
	-v_init -1 \
	-dpos_init 0 \
	-dneg_init 0 \
	-mode_swt 1 \
	-h_swt -1 \
	-v_swt 1 \
	-dpos_swt 0 \
	-dneg_swt 0 \
	-train_time 0 \
	-switch_time1 0 \
	-switch_time2 0 \
	-suffix $${i} \
	;)&\
	done


simulated_sleep:
	for percentIncrease in 0.6 0.9 1.2 1.5; do( \
	for threshold in 0.01 0.025 0.05 0.075 0.1; do( \
	for time in 2 4 6 8 10; do( \
	for i in 1 2 3 4 5; do( \
	mkdir percInc$${percentIncrease}_threshold$${threshold}; \
	./$(SpiritExecutableFileName) \
	-output_location percInc$${percentIncrease}_threshold$${threshold}/out$${i} \
	-mode_init 0 \
	-h_init 1 \
	-v_init 0 \
	-dpos_init -1 \
	-dneg_init 0 \
	-mode_swt 0 \
	-h_swt 0 \
	-v_swt 1 \
	-dpos_swt 0 \
	-dneg_swt -1 \
	-train_time 0 \
	-switch_time1 0 \
	-switch_time2 0 \
	-suffix $${i} \
	-percentWeightIncrease $${percentIncrease} \
	-thresholdWeightIncrease $${threshold} \
	-timeInterleavedTraining $${time} \
	;)& done \
	;) done \
	;) done \
	;) done 

standard:
	for con in 1; do( \
	for i in 1; do( \
	./$(SpiritExecutableFileName) \
	-output_location pop$${con}/out$${i} \
	-init_network $(ConnectivityFolder)/$${con}.conn \
	-init_stages stageConfigs/TrainingStages.config \
	-pubPrivPrivateStepInputLayerThreshold 0 \
	-pubPivPublicStepInputLayerThreshold 0.1 \
	-suffix $${i} \
	;) done \
	;) done
# __________________________Sims used for paper Start__________________________________

train1:
	for con in 1; do( \
	for i in 1 2 3 4 5 6 7 8 9 10; do( \
	./$(SpiritExecutableFileName) \
	-output_location train1/out$${i} \
	-init_network $(ConnectivityFolder)/$${con}.conn \
	-init_stages stageConfigs/train1.config \
	-suffix $${i} \
	;)& done \
	;) done

train2:
	for con in 1; do( \
	for i in 1 2 3 4 5 6 7 8 9 10; do( \
	./$(SpiritExecutableFileName) \
	-output_location train2/out$${i} \
	-init_network $(ConnectivityFolder)/$${con}.conn \
	-init_stages stageConfigs/train2.config \
	-suffix $${i} \
	;)& done \
	;) done

train1InterleavedNoise:
	for con in 1; do( \
	for i in 1 2 3 4 5 6 7 8 9 10; do( \
	./$(SpiritExecutableFileName) \
	-output_location train1InterleavedNoise/out$${i} \
	-init_network $(ConnectivityFolder)/$${con}.conn \
	-init_stages stageConfigs/train1InterleavedNoise.config \
	-suffix $${i} \
	;)& done \
	;) done

train1InterleavedTraining:
	for con in 1; do( \
	for i in 1 2 3 4 5 6 7 8 9 10; do( \
	./$(SpiritExecutableFileName) \
	-output_location train1InterleavedTraining/out$${i} \
	-init_network $(ConnectivityFolder)/$${con}.conn \
	-init_stages stageConfigs/train1InterleavedTraining.config \
	-suffix $${i} \
	;)& done \
	;) done

train1Train2Train1:
	for con in 1; do( \
	for i in 1 2 3 4 5 6 7 8 9 10; do( \
	./$(SpiritExecutableFileName) \
	-output_location train1Train2Train1/out$${i} \
	-init_network $(ConnectivityFolder)/$${con}.conn \
	-init_stages stageConfigs/train1Train2Train1.config \
	-suffix $${i} \
	;)& done \
	;) done

train1Train2FlipInterleavedNoise:
	for con in 1; do( \
	for i in 1 2 3 4 5 6 7 8 9 10; do( \
	./$(SpiritExecutableFileName) \
	-output_location train1Train2FlipInterleavedNoise/out$${i} \
	-init_network $(ConnectivityFolder)/$${con}.conn \
	-init_stages stageConfigs/flipTask1Task2-5-InterleavedNoise.config \
	-suffix $${i} \
	;)& done \
	;) done

train1Train2FlipInterleavedTraining:
	for con in 1; do( \
	for i in 1 2 3 4 5 6 7 8 9 10; do( \
	./$(SpiritExecutableFileName) \
	-output_location train1Train2FlipInterleavedTraining/out$${i} \
	-init_network $(ConnectivityFolder)/$${con}.conn \
	-init_stages stageConfigs/flipTask1Task2-5-InterleavedTraining.config \
	-suffix $${i} \
	;)& done \
	;) done

interleavedTraining:
	for con in 1; do( \
	for i in 1 2 3 4 5 6 7 8 9 10; do( \
	./$(SpiritExecutableFileName) \
	-output_location interleavedTraining/out$${i} \
	-init_network $(ConnectivityFolder)/$${con}.conn \
	-init_stages stageConfigs/interleavedTraining.config \
	-suffix $${i} \
	;)& done \
	;) done

interleavedNoise:
	for con in 1; do( \
	for i in 1 2 3 4 5 6 7 8 9 10; do( \
	./$(SpiritExecutableFileName) \
	-output_location interleavedNoise/out$${i} \
	-init_network $(ConnectivityFolder)/$${con}.conn \
	-init_stages stageConfigs/interleavedNoise.config \
	-suffix $${i} \
	;)& done \
	;) done

interleavedUniformNoise:
	for con in 1; do( \
	for i in 1 2 3 4 5 6 7 8 9 10; do( \
	./$(SpiritExecutableFileName) \
	-output_location interleavedUniformNoise/out$${i} \
	-init_network $(ConnectivityFolder)/$${con}.conn \
	-init_stages stageConfigs/t1-t2-t1-interleavedUniformNoise.config \
	-suffix $${i} \
	;)& done \
	;) done

train1FreezeTopTrain2:
	for con in 1; do( \
	for i in 1 2 3 4 5 6 7 8 9 10; do( \
	./$(SpiritExecutableFileName) \
	-output_location train1FreezeTopTrain2/out$${i} \
	-init_network $(ConnectivityFolder)/$${con}.conn \
	-init_stages stageConfigs/train1FreezeTopTrain2.config \
	-suffix $${i} \
	;)& done \
	;) done

# __________________________Sims used for paper End__________________________________
bump: 
	for con in 1; do( \
        for thresh in 0.0205 0.021 0.02105; do( \
        for bump in 0.5 0.6; do( \
        for perf in 0.55 0.6 0.65; do( \
        for i in 1 2 3 4 5; do( \
        ./$(SpiritExecutableFileName) \
        -output_location pop$${con}/thresh$${thresh}bump$${bump}perf$${perf}/out$${i} \
        -init_network $(ConnectivityFolder)/$${con}.conn \
        -mode_init 0 \
        -h_init 1 \
        -v_init 0 \
        -dpos_init -1 \
        -dneg_init 0 \
        -mode_swt 0 \
        -h_swt 0 \
        -v_swt 1 \
        -dpos_swt 0 \
        -dneg_swt -1 \
        -thresholdWeightIncrease $${thresh} \
        -percentWeightIncrease $${bump} \
	-train1TillPerformanceThreshold $${perf} \
        -suffix $${i} \
        ;)& done \
        ;) done \
        ;) done \
        ;) done

poissonEpochInputLow: 
	for con in 1; do( \
        for perf in 0.55; do( \
        for poissonThresh in 0.1 0.2 0.3 0.4 0.5 0.6; do( \
        for i in 1 2 3 4 5; do( \
        ./$(SpiritExecutableFileName) \
        -output_location pop$${con}/poissonThresh$${poissonThresh}perf$${perf}/out$${i} \
        -init_network $(ConnectivityFolder)/$${con}.conn \
	-init_stages stageConfigs/PoissonInputNoise.config \
        -poissonEpochInputLayerThreshold $${poissonThresh} \
	-train1TillPerformanceThreshold $${perf} \
        -suffix $${i} \
        ;)& done \
        ;) done \
        ;) done \
        ;) done

poissonEpochInputHigh: 
	for con in 1; do( \
        for perf in 0.65; do( \
        for poissonThresh in 0.1 0.2 0.3 0.4 0.5 0.6; do( \
        for i in 1 2 3 4 5; do( \
        ./$(SpiritExecutableFileName) \
        -output_location pop$${con}/poissonThresh$${poissonThresh}perf$${perf}/out$${i} \
        -init_network $(ConnectivityFolder)/$${con}.conn \
	-init_stages stageConfigs/PoissonInputNoise.config \
        -poissonEpochInputLayerThreshold $${poissonThresh} \
	-train1TillPerformanceThreshold $${perf} \
        -suffix $${i} \
        ;)& done \
        ;) done \
        ;) done \
        ;) done

poissonStepInputLow: 
	for con in 1; do( \
        for perf in 0.55; do( \
        for poissonThresh in 0.05 0.1 0.125 0.15 0.2; do( \
        for i in 1 2 3 4 5; do( \
        ./$(SpiritExecutableFileName) \
        -output_location pop$${con}/poissonThresh$${poissonThresh}perf$${perf}/out$${i} \
        -init_network $(ConnectivityFolder)/$${con}.conn \
	-init_stages stageConfigs/PoissonInputNoise.config \
        -poissonStepInputLayerThreshold $${poissonThresh} \
	-train1TillPerformanceThreshold $${perf} \
        -suffix $${i} \
        ;)& done \
        ;) done \
        ;) done \
        ;) done

poissonStepInputHigh: 
	for con in 1; do( \
        for perf in 0.65; do( \
        for poissonThresh in 0.05 0.1 0.125 0.15 0.2; do( \
        for i in 1 2 3 4 5; do( \
        ./$(SpiritExecutableFileName) \
        -output_location pop$${con}/poissonThresh$${poissonThresh}perf$${perf}/out$${i} \
        -init_network $(ConnectivityFolder)/$${con}.conn \
	-init_stages stageConfigs/PoissonInputNoise.config \
        -poissonStepInputLayerThreshold $${poissonThresh} \
	-train1TillPerformanceThreshold $${perf} \
        -suffix $${i} \
        ;)& done \
        ;) done \
        ;) done \
        ;) done

foodNoise: 
	for con in 1; do( \
	for perf in 0.55 0.6 0.65 0.7 0.75; do( \
	for i in 1 2 3 4 5; do( \
	./$(SpiritExecutableFileName) \
	-output_location pop$${con}foodNoise/perf$${perf}/out$${i} \
	-init_network $(ConnectivityFolder)/$${con}.conn \
	-init_stages stageConfigs/FoodNoise.config \
	-train1TillPerformanceThreshold $${perf} \
	-suffix $${i} \
	;)& done \
	;) done \
	;) done 

pubPrivNoise: 
	for con in 1; do( \
	for perf in 0.65 ; do( \
        for publicThresh in 0.1 0.3; do( \
        for privateThresh in 0.1 0.3; do( \
	for i in 1 2 3 4 5; do( \
	./$(SpiritExecutableFileName) \
	-output_location pop$${con}publicPrivateNoise/perf$${perf}publicThresh$${publicThresh}privateThresh$${privateThresh}/out$${i} \
	-init_network $(ConnectivityFolder)/$${con}.conn \
	-init_stages stageConfigs/PubPrivNoise.config \
	-train1TillPerformanceThreshold $${perf} \
	-pubPrivPrivateStepInputLayerThreshold $${privateThresh} \
	-pubPivPublicStepInputLayerThreshold $${publicThresh} \
	-suffix $${i} \
	;)& done \
	;) done \
	;) done \
	;) done \
	;) done 

poissonByNeuron: 
	for con in 1; do( \
        for perf in 0.65; do( \
        for i in 1 2 3 4 5 6 7 8 9 10; do( \
        ./$(SpiritExecutableFileName) \
        -output_location poissonByNeuron/perf$${perf}/out$${i} \
        -init_network $(ConnectivityFolder)/$${con}.conn \
	-init_stages stageConfigs/PoissonByNeuron.config \
	-train1TillPerformanceThreshold $${perf} \
        -suffix $${i} \
        ;)& done \
        ;) done \
        ;) done

twoTaskPoissonByNeuron: 
	for con in 1; do( \
        for perf1 in 0.7; do( \
        for perf2 in 0.7; do( \
        for i in 1 2 3 4; do( \
        ./$(SpiritExecutableFileName) \
        -output_location poissonByNeuronTwoTask/perf1$${perf1}perf2$${perf2}/out$${i} \
        -init_network $(ConnectivityFolder)/$${con}.conn \
        -init_stages stageConfigs/TwoTaskPoissonByNeuron.config \
        -train1TillPerformanceThreshold $${perf1} \
        -train2TillPerformanceThreshold $${perf2} \
        -suffix $${i} \
        ;)& done \
        ;) done \
        ;) done \
        ;) done

iboSro:
	for con in 1; do( \
	for sro in 0 1; do( \
	for ibo in 0 1; do( \
	for i in 1 2 3 4 5 6; do( \
	./$(SpiritExecutableFileName) \
	-output_location pop$${con}/sro$${sro}ibo$${ibo}/out$${i} \
	-init_network $(ConnectivityFolder)/$${con}.conn \
	-init_stages stageConfigs/TrainingStages.config \
	-enableSpikeRateAdaptationOverride $${sro} \
	-enableInputBalencingOverride $${ibo} \
	-suffix $${i} \
	;)& done \
	;) done \
	;) done \
	;) done

ibSr:
	for con in 1; do( \
	for sr in 0 1; do( \
	for ib in 0 1; do( \
	for i in 1 2 3 4 5 6; do( \
	./$(SpiritExecutableFileName) \
	-output_location pop$${con}/trainingSr$${sr}trainingIb$${ib}/out$${i} \
	-init_network $(ConnectivityFolder)/$${con}.conn \
	-init_stages stageConfigs/TrainingStages.config \
	-trainingSpikeRateAdaptation $${sr} \
	-trainingInputBalencing $${ib} \
	-suffix $${i} \
	;)& done \
	;) done \
	;) done \
	;) done

poissonByNeuronParams:
	for con in 1; do( \
	for al in 3.65 3.625 3.6; do( \
	for sig in 0.06 0.055 0.05; do( \
	for wScale in 1.5 2; do( \
	for i in 1 2 3 4 5; do( \
	./$(SpiritExecutableFileName) \
	-output_location poissonByNeuronParams/al$${al}sig$${sig}wScale$${wScale}/out$${i} \
	-init_network $(ConnectivityFolder)/$${con}.conn \
	-init_stages stageConfigs/PoissonByNeuron.config \
	-cellParamNoiseAl $${al} \
	-cellParamNoiseSig $${sig} \
	-noiseWeightScaleFactor $${wScale} \
	-suffix $${i} \
	;)& done \
	;) done \
	;) done \
	;) done \
	;) done

bugBrain:
	for con in 1; do( \
	for initHidNeurons in 25 50 100 200 400 600 784; do( \
	for i in 1 2 3 4 5; do( \
	./$(SpiritExecutableFileName) \
	-output_location pop$${con}/numberHidNeurons$${initHidNeurons}/out$${i} \
	-init_network $(ConnectivityFolder)/$${con}.conn \
	-init_stages stageConfigs/TrainingStages.config \
	-initialNumberHiddenLayerNeurons $${initHidNeurons} \
	-suffix $${i} \
	;) done \
	;) done \
	;) done
	
interleavedSleepTrain2: 
	for con in 1; do( \
        for i in 1 2 3 4 5; do( \
        ./$(SpiritExecutableFileName) \
        -output_location interleavedSleepTrain2/out$${i} \
        -init_network $(ConnectivityFolder)/$${con}.conn \
	-init_stages stageConfigs/InterleavedNoiseTrainTask2.config \
        -suffix $${i} \
        ;)& done \
        ;) done

