#include "dma.h"

#include <unistd.h>  // needed for getpagesize()

#include "rfmdriver.h"
#include "modules/zmq/logger.h"

DMA::DMA()
    : m_memory(NULL)
{
    m_status = new t_status;
}

DMA::~DMA()
{
    delete m_status,
           m_memory; // Not sure if this one should be deleted
}

int DMA::init(RFMDriver *driver)
{
    if (driver == NULL)
        return 1;

    RFM2G_UINT32 rfm2gSize(0);
    volatile void *pPioCard = NULL;

    Logger::Logger() << "RFM DMA Init";

    driver->setDMAThreshold(DMA_THRESHOLD);

    RFM2GCONFIG rfm2gConfig;
    RFM2G_STATUS getConfigError = driver->getConfig(&rfm2gConfig);
    if (getConfigError == RFM2G_SUCCESS) {
        pPioCard = (char*)rfm2gConfig.PciConfig.rfm2gBase;
        rfm2gSize = rfm2gConfig.PciConfig.rfm2gWindowSize;
    }

    int pageSize = getpagesize();
    unsigned int numPagesDMA = rfm2gSize / (2* pageSize);
    unsigned int numPagesPIO = rfm2gSize / (2* pageSize);
    if ((rfm2gSize % pageSize) > 0) {
        Logger::Logger() << "\tIncrease PIO and DMA";
        numPagesDMA++;
        numPagesPIO++;
    }

    Logger::Logger() << "\tpPioCard : " << pPioCard;
    Logger::Logger() << "\trfm2gSize : " << rfm2gSize;
    Logger::Logger() << "\tpageSize  : " << pageSize;
    Logger::Logger() << "\tnumPages DMA/PIO : " << numPagesDMA;

    RFM2G_STATUS rfmMemoryError = driver->userMemory(
                                        (volatile void **) (&m_memory),
                                        (DMAOFF_A | LINUX_DMA_FLAG),
                                        numPagesDMA
                                        );
    if (rfmMemoryError) {
        Logger::error(_ME_) << "doDMA: ERROR: Failed to map card DMA buffer; "
                            << driver->errorMsg(rfmMemoryError);
        return -1;
    }

    Logger::Logger() << "doDMA: SUCCESS: mapped numPagesDMA=" << numPagesDMA
                       << " at pDmaCard=" << std::hex << m_memory;

    rfmMemoryError = driver->userMemoryBytes((volatile void **) (&pPioCard),
                                             (0x00000000 | LINUX_DMA_FLAG2),
                                             rfm2gSize);
    if (rfmMemoryError) {
        Logger::error(_ME_) << "doDMA: ERROR: Failed to map card PIO; "
                            << driver->errorMsg(rfmMemoryError);

        return -1;
    }

    Logger::Logger() << "doDMA: Card: PIO memory pointer = 0x" << std::hex << pPioCard
                       << ", Size = 0x" << std::hex << rfm2gSize;

    return 0;
}
