nBands = 3; % Number of EQ bands
nStages = 2; % Number of stages of each band
dBmax = 6; % Max dB
dBmin = -6; % Min dB
nBits = 15; % 31
Fs    = 48e3;
fMin = 50;
fMax = 15000;


nSteps = dBmax-dBmin+1;
freqs = logspace(log10(fMin),log10(fMax),nBands+2);
freqs = freqs(2:end-1);
N     = 2*nStages;

close all;
figure;
set(gca,'xscale','log');
hold on

eqDefsFile = fopen('EqualizerDefs.h','w');

fprintf(eqCoeffsFile,'#include <arm_math.h>\n\n\n');
fprintf(eqCoeffsFile,'#define EQ_NUM_STEPS \t\t %dU \n\n',nSteps);
fprintf(eqCoeffsFile,'#define EQ_NUM_COEFFS \t\t %dU \n\n',6*nStages);
fprintf(eqCoeffsFile,'#define EQ_NUM_BANDS \t\t %dU \n\n',nBands);
fprintf(eqCoeffsFile,'#define EQ_NUM_STAGES \t\t %dU \n\n',nStages);
fprintf(eqCoeffsFile,'#define EQ_MAX_DB \t\t %d \n\n',dBmax);
fprintf(eqCoeffsFile,'#define EQ_MIN_DB \t\t %d \n\n',dBmin);

fprintf(eqCoeffsFile,'extern const q15_t eqCoeffs[EQ_NUM_BANDS * EQ_NUM_STEPS * EQ_NUM_COEFFS]; \n\n');

fprintf(eqCoeffsFile,'static inline q15_t * getCoeffs(uint8_t band, int8_t level)\n');
fprintf(eqCoeffsFile,'{\n');
fprintf(eqCoeffsFile,'\treturn &eqCoeffs[EQ_NUM_STEPS*EQ_NUM_COEFFS*band + EQ_NUM_COEFFS*(level - EQ_MIN_DB)];\n');
fprintf(eqCoeffsFile,'}\n');

fclose(eqDefsFile);

eqCoeffsFile = fopen('EqualizerCoeffs.c','w');

fprintf(eqCoeffsFile,'#include "EqualizerDefs.h" \n\n\n');

fprintf(eqCoeffsFile,"const q15_t eqCoeffs[EQ_NUM_BANDS * EQ_NUM_STEPS * EQ_NUM_COEFFS] = \n{\n");

for i=1:nBands
%     if i==1 || i==nBands
%         % Shelving equalizer
%         slope = 1;
%         if i==1
%             Fc = freqs(i)/(Fs/2);
%             type = 'lo';
%         elseif i==nBands
%             Fc = freqs(i)/(Fs/2);
%             type = 'hi';
%         end
%         
%         coeffs = [];
%         % Iterate for every possible gain
%         for G = dBmin:1:dBmax
%             
%             [B,A] = designShelvingEQ(G/nStages,slope,Fc,type);
%             
%             A=A';
%             B=B';
%             
%             SOS   = [B ones(sum(2)/2,1) A];
%             SOS = repmat(SOS,nStages,1);
%             
%             [m,f] = freqz(dsp.BiquadFilter(SOS),5000,Fs);
%             
%             plot(f,20*log10(abs(m)));
%             
%             SOS = [B(:,1) zeros(sum(2)/2,1) B(:,2:3) -A(:,1:2)]; % ESTE PUTO MENOS!!!!
%             SOS = repmat(SOS,nStages,1);
%             
%             coeffs = [reshape(SOS',1,[]); coeffs];
%         end
%         
%         
%     else
        % Peaking equalizer
        Noct  = (log2(fMax)-log2(fMin))/nBands;
        Q     = sqrt(2.^Noct)./(2.^Noct-1); % Q factors
        Wo    = freqs(i)/(Fs/2);
        BW    = Wo./Q;
        
        coeffs = [];
        % Iterate for every possible gain
        for G = dBmin:1:dBmax
            
            [B,A] = designParamEQ(N,G,Wo,BW);
            
            A=A';
            B=B';
            
            SOS   = [B ones(sum(N)/2,1) A];
            
            [m,f] = freqz(dsp.BiquadFilter(SOS),5000,Fs);
            
            plot(f,20*log10(abs(m)));
            
            SOS = [B(:,1) zeros(sum(N)/2,1) B(:,2:3) -A(:,1:2)]; % ESTE PUTO MENOS!!!!
            
            coeffs = [reshape(SOS',1,[]); coeffs];
        end
%    end
    
    coeffs = single(coeffs)/2;
    
    coeffsQ15 = fi(coeffs,1,0,15);
    
    nCoeffs = 6*nStages;
    
    if size(coeffs,1) ~= nSteps || size(coeffs,2) ~= nCoeffs
        error('Error with dimentions');
    end
    
    % Print filter
    for k = 1:size(coeffsQ15,1)
        fprintf(eqCoeffsFile,"\t");
        for j=1:size(coeffsQ15,2)-1
            fprintf(eqCoeffsFile,"0x%s, ",hex(coeffsQ15(k,j)));
        end
        if k==size(coeffsQ15,1) && i == nBands
            fprintf(eqCoeffsFile,"0x%s\n",hex(coeffsQ15(k,size(coeffsQ15,2))));
        else
            fprintf(eqCoeffsFile,"0x%s,\n",hex(coeffsQ15(k,size(coeffsQ15,2))));
        end
    end
end

fprintf(eqCoeffsFile,"};\n\n");
%
%  fprintf(eqCoeffsFile,"const q15_t coeffs[%s][%s] = \n{\n",name,dim1,dim2);
%     for i=1:size(arr,1)
%         fprintf(eqCoeffsFile,"\t{");
%         for j=1:size(arr,2)-1
%             fprintf(eqCoeffsFile,"0x%s, ",hex(arr(i,j)));
%         end
%         if i<size(arr,1)
%             fprintf(eqCoeffsFile,"0x%s},\n",hex(arr(i,size(arr,2))));
%         else
%             fprintf(eqCoeffsFile,"0x%s}\n};\n",hex(arr(i,size(arr,2))));
%         end
%     end


fclose(eqCoeffsFile);

% plot(freqs, 2*ones(nBands,1),'o')
xlim([fMin fMax]);

