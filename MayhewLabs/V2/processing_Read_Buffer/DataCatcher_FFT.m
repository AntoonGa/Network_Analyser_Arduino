% MATLAB FFT ROUTINE
% Setup when run: Give ZeroPaddingSize and ADC chip name (only MCP3002
% available for now).
% Workflow:
%1)Loads buffer file #.txt ONLY IF NEW AVAILABLE
%2)Computes Spectral Width, resolution etc.
%3)Computes and draw FFT.
%-You don't have to stop this program, it should grab new files without you
%doing anything.
%-Can run when other task (arduino/processing) are running
%-Can be pause/resumed
%-Still not super stable, has to be relauched (run button) from time to
%time. Must add more trycatch routines



%Init parameters and some options, nothing interesting
for ii=1:1
close all; clc; disp('starting');
clear;
lastIndex=-1;
%config.ADCmodel='MCP3002'; %adc reference: see switch case!
config.ADCmodel='Mayhews';
config.Decay = 0; %artifical decay 0 for no, 1 for yes...
config.ZeroPadingSize=10;	%ZeroPading buffer multiplicator (must be positive integer);
config.FreqCorrection=0.999717324939195; %manual correction 
config.TimeCorrection=0;                %manual correction
tic      %solves stupid tictoc stuff do not remove
end

%ADC config switch case, nothing interesting
switch config.ADCmodel
        case 'MCP3002' 
    Vdd=5;                             %Voltage reference
    config.ADCconversionCoef=Vdd/1024; %ADC conversion formula, see datasheet
        case 'Mayhews'
    config.ADCconversionCoef=10/(65535);
        otherwise
    config.ADCconversionCoef=1;        %ADC conversion formula, see datasheet
end

%main(): loadbuffer trycatch loop, fft, plot
while 1
    
clearvars -except lastIndex filename d config Thebuffer; 
%get lastfile name via windows10 writing time.
d=dir('*.txt');     
dates=[d.datenum];
[~, newestIndex]=max(dates);
    
%load only if newfile, then execute FFT
    if newestIndex == lastIndex
    else
        filename = [num2str(newestIndex) , '.txt'];

        %TryCatch load subroutine (some error on firsts files)
        while 1
            try
                Thebuffer=load(filename);
                disp(['loaded: ',filename]);
                break;
            catch 
                fprintf('error loading, retry...');
                pause(0.2);           
            end
        end

        %main code if buffer is readable
        if isempty(Thebuffer)
            disp('error reading, retry...');
            pause(0.5);
        else
            %split time stamp and samples; get sample frequency
            N=numel(Thebuffer)-1;
            TotTime=Thebuffer(end);
            T=(TotTime+config.TimeCorrection)/(N)*1e-6;
            Fs=1/T*(1/config.FreqCorrection); %use calibration
            T=1/Fs; %recalibrate time vector
            Thebuffer=Thebuffer(1:N);

            %ADC voltage conversion coefficient
            Thebuffer=Thebuffer.*config.ADCconversionCoef;

            %FFT subroutine
            L=N*config.ZeroPadingSize;	%buffer size for FFT (with ZeroParding)
            xx=linspace(0,TotTime,N); %Time vector
            
            decayVector=linspace(0,3,N);
            decay=exp(-decayVector)';
            %FFT 
            %Thebuffer = detrend(Thebuffer);    %removes DC component (zero Frequency peak)                  
            %Thebuffer=Thebuffer-mean(Thebuffer);
            
            if config.Decay==1
                decayVector=linspace(0,3,N);
                decay=exp(-decayVector)';
                Thebuffer=Thebuffer.*decay;
            end
            Y = fft(Thebuffer,L);                 
            P2 = abs(Y/L);
            P1 = P2(1:L/2+1); %Final FFT values
            f = Fs*(0:L/2)/(L); %Frequency vector
            
            %get max intensity and corresponding frequency
            [FFTmax, TheIndex] = max(P1); 
            Freq=f(TheIndex);
            %plot subroutine
           figure(1)                   
           subplot(5,1,1);
            plot(xx(1:1000), Thebuffer(1:1000)); %only 200pts are ploted for readability
            hold on;
            title([filename,' | Fs= ',num2str(Fs),' | dF= ',num2str(Fs/L),' Hz | N=',num2str(N)]);
            ylabel(['FID [V] | max V=', num2str(max(Thebuffer))])
            hold off;
           subplot(5,1,2);
            plot(f,P1);
            hold on;
            title(['Peak frequency= ',num2str(Freq),' Hz']);
            ylabel('(FFT(FID))')
            xlim([0 max(f)])
            hold off;
            subplot(5,1,3);
           plot(f,20*log10(P1));            
            hold on;
            title(['log(fft)']);
            ylabel('Log(FFT(FID))')
            xlim([0 max(f)])
            ylim([min(20*log10(P1)) max(20*log10(P1))])
            hold off;
%             %SNR, removes the 60hz signal.
% Filter = designfilt('bandstopiir','FilterOrder',2, ...
%                'HalfPowerFrequency1',59,'HalfPowerFrequency2',61, ...
%                'DesignMethod','butter','SampleRate',Fs);                       
% Filtered = filtfilt(Filter,Thebuffer);                        
            subplot(5,1,4);
            hold on;
            %snr(Thebuffer,Fs,10);
            sinad(Thebuffer,Fs)
            %xlabel('Frequency [Hz]')
            hold off;
            subplot(5,1,5);
            hold on;
            thd(Thebuffer,Fs,100);
            %xlabel('Frequency [Hz]')
            hold off;
            drawnow;
            toc
            tic
        end   
    end    
lastIndex=newestIndex;

end


