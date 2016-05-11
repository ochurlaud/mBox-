#include "handlers/correction/correctionhandler.h"

#include "define.h"
#include "adc.h"
#include "dac.h"
#include "dma.h"
#include "modules/zmq/logger.h"

CorrectionHandler::CorrectionHandler(RFMDriver *driver, DMA *dma, bool weightedCorr)
    : Handler(driver, dma, weightedCorr)
{
}

int CorrectionHandler::typeCorrection()
{
    int typeCorr = Correction::None;
    if ((m_plane == 0) || (m_plane == 1) || ((m_plane == 3) && (m_loopDir > 0))) {
        typeCorr |= Correction::Horizontal ;
    }
    if ((m_plane == 0) || (m_plane == 2) || ((m_plane == 3) && (m_loopDir < 0))) {
        typeCorr |= Correction::Vertical;
    }
    return typeCorr;
}

int CorrectionHandler::callProcessorRoutine(const CorrectionInput_t& input,
                                            arma::vec& CMx, arma::vec& CMy)
{
    return m_correctionProcessor.correct(input, CMx, CMy);
}

void CorrectionHandler::setProcessor(arma::mat SmatX, arma::mat SmatY,
                                     double IvecX, double IvecY,
                                     double Frequency,
                                     double P, double I, double D,
                                     arma::vec CMx, arma::vec CMy,
                                     bool weightedCorr)
{
    m_correctionProcessor.setCMs(CMx, CMy);
    m_correctionProcessor.setSmat(SmatX, SmatY, IvecX, IvecY, weightedCorr);
    m_correctionProcessor.setInjectionCnt(Frequency);
    m_correctionProcessor.setPID(P,I,D);
}


