
#include "base/base_crc.h"
#include "base/base_exception.h"
#include "base/base_managed_pool.h"
#include "base/base_uncopyable.h"
#include "base/base_singleton.h"

#include "mr/mr_marker_manager.h"

#include <AR/ar.h>

#include <unordered_map>

using namespace MR;

namespace
{
	std::string g_PathToAssets = "../assets/";
} // namespace

namespace
{
    class CMarkerManager : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CMarkerManager);
        
    public:
        
        CMarkerManager();
        ~CMarkerManager();
        
    public:
        
        void OnStart();
        void OnExit();

        void Clear();
        
    public:
        
        CMarkerPtr CreateMarker(const SMarkerDescription& _rDescription);
        
        CMarkerPtr GetMarkerByID(unsigned int _ID);
        
    private:
        
        class CInternMarker : public CMarker
        {
        public:
            
            CInternMarker();
            ~CInternMarker();
            
        private:
            
            friend class CMarkerManager;
        };
        
    private:
        
        typedef Base::CManagedPool<CInternMarker, 8, 1> CMarkers;
        
        typedef CMarkers::CIterator CMarkerIterator;
        
        typedef std::unordered_map<unsigned int, CInternMarker*> CMarkerByHashs;
        
    private:
        
        CMarkers       m_Markers;
        CMarkerByHashs m_MarkerByHashs;
        CMarkerByHashs m_MarkerByIDs;
    };
} // namespace

namespace
{
    CMarkerManager::CInternMarker::CInternMarker()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    CMarkerManager::CInternMarker::~CInternMarker()
    {
        if (m_pHandle != 0)
        {
            ARPattHandle* pNativeHandle = static_cast<ARPattHandle*>(m_pHandle);
            
            arPattDeleteHandle(pNativeHandle);
        }
    }
} // namespace

namespace
{
    CMarkerManager::CMarkerManager()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    CMarkerManager::~CMarkerManager()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CMarkerManager::OnStart()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CMarkerManager::OnExit()
    {
        Clear();
    }

    // -----------------------------------------------------------------------------

    void CMarkerManager::Clear()
    {
        // -----------------------------------------------------------------------------
        // Clear all markers
        // -----------------------------------------------------------------------------
        m_Markers.Clear();

        // -----------------------------------------------------------------------------
        // Clear all hashes
        // -----------------------------------------------------------------------------
        m_MarkerByHashs.clear();
        m_MarkerByIDs.clear();
    }
    
    // -----------------------------------------------------------------------------

    CMarkerPtr CMarkerManager::CreateMarker(const SMarkerDescription& _rDescription)
    {
#pragma warning( disable : 4706 4996 )
        auto LoadPatternFromBuffer = [&](ARPattHandle *pattHandle, const char *buffer) 
        {
            char   *bufCopy;
            int     patno;
            int     h, i1, i2, i3;
            int     i, j, l, m;
            char   *buffPtr;
            const char *delims = " \t\n\r";

            if (!pattHandle) {
                ARLOGe("Error: NULL pattHandle.\n");
                return (-1);
            }
            if (!buffer) {
                ARLOGe("Error: can't load pattern from NULL buffer.\n");
                return (-1);
            }

            for (i = 0; i < pattHandle->patt_num_max; i++) {
                if (pattHandle->pattf[i] == 0) break;
            }
            if (i == pattHandle->patt_num_max) return -1;
            patno = i;

            if (!(bufCopy = _strdup(buffer))) { // Make a mutable copy.
                ARLOGe("Error: out of memory.\n");
                return (-1);
            }
            buffPtr = strtok(bufCopy, delims);

            for (h = 0; h<4; h++) {
                l = 0;
                for (i3 = 0; i3 < 3; i3++) { // Three colours B G R
                    for (i2 = 0; i2 < pattHandle->pattSize; i2++) { // Rows
                        for (i1 = 0; i1 < pattHandle->pattSize; i1++) { // Columns

                            /* Switch file scanning to buffer reading */

                            /* if( fscanf(fp, "%d", &j) != 1 ) {
                            ARLOGe("Pattern Data read error!!\n");
                            return -1;
                            }
                            */

                            if (buffPtr == NULL) {
                                ARLOGe("Pattern Data read error!!\n");
                                free(bufCopy);
                                return -1;
                            }

                            j = atoi(buffPtr);
                            buffPtr = strtok(NULL, delims);

                            j = 255 - j;
                            pattHandle->patt[patno * 4 + h][(i2*pattHandle->pattSize + i1) * 3 + i3] = j;
                            if (i3 == 0) pattHandle->pattBW[patno * 4 + h][i2*pattHandle->pattSize + i1] = j;
                            else          pattHandle->pattBW[patno * 4 + h][i2*pattHandle->pattSize + i1] += j;
                            if (i3 == 2) pattHandle->pattBW[patno * 4 + h][i2*pattHandle->pattSize + i1] /= 3;
                            l += j;
                        }
                    }
                }
                l /= (pattHandle->pattSize*pattHandle->pattSize * 3);

                m = 0;
                for (i = 0; i < pattHandle->pattSize*pattHandle->pattSize * 3; i++) {
                    pattHandle->patt[patno * 4 + h][i] -= l;
                    m += (pattHandle->patt[patno * 4 + h][i] * pattHandle->patt[patno * 4 + h][i]);
                }
                pattHandle->pattpow[patno * 4 + h] = sqrt((ARdouble)m);
                if (pattHandle->pattpow[patno * 4 + h] == 0.0) pattHandle->pattpow[patno * 4 + h] = 0.0000001;

                m = 0;
                for (i = 0; i < pattHandle->pattSize*pattHandle->pattSize; i++) {
                    pattHandle->pattBW[patno * 4 + h][i] -= l;
                    m += (pattHandle->pattBW[patno * 4 + h][i] * pattHandle->pattBW[patno * 4 + h][i]);
                }
                pattHandle->pattpowBW[patno * 4 + h] = sqrt((ARdouble)m);
                if (pattHandle->pattpowBW[patno * 4 + h] == 0.0) pattHandle->pattpowBW[patno * 4 + h] = 0.0000001;
            }

            free(bufCopy);

            pattHandle->pattf[patno] = 1;
            pattHandle->patt_num++;

            return(patno);
        };

        auto LoadPattern = [&](ARPattHandle *pattHandle, const char *filename)
        {
            FILE   *fp;
            int     patno;
            size_t  ret;

            /* Old variables */
            /*
            int     h, i1, i2, i3;
            int     i, j, l, m;
            */

            /* New variables */
            long pos = 0;
            char* bytes = NULL;

            /* Open file */
            fp = fopen(filename, "rb");
            if (fp == NULL) {
                ARLOGe("Error opening pattern file '%s' for reading.\n", filename);
                return (-1);
            }

            /* Determine number of bytes in file */
            fseek(fp, 0L, SEEK_END);
            pos = ftell(fp);
            fseek(fp, 0L, SEEK_SET);

            //ARLOGd("Pattern file is %ld bytes\n", pos);

            /* Allocate buffer */
            bytes = (char *)malloc(pos + 1);
            if (!bytes) {
                ARLOGe("Out of memory!!\n");
                fclose(fp);
                return (-1);
            }

            /* Read pattern into buffer and close file */
            ret = fread(bytes, pos, 1, fp);
            fclose(fp);
            if (ret < 1) {
                ARLOGe("Error reading pattern file '%s'.\n", filename);
                free(bytes);
                return (-1);
            }

            /* Terminate string */
            bytes[pos] = '\0';

            /* Load pattern from buffer */
            patno = LoadPatternFromBuffer(pattHandle, bytes);

            /* Free allocated buffer */
            free(bytes);

            return(patno);
        };
#pragma warning( error : 4706 4996 )


        unsigned int Hash = 0;
        
        // -----------------------------------------------------------------------------
        // Build path to texture in file system
        // -----------------------------------------------------------------------------
        std::string PathToPattern;
        
        // -----------------------------------------------------------------------------
        // Create hash and check if marker already available
        // -----------------------------------------------------------------------------
        const char* pPatternFile = _rDescription.m_pPatternFile;
        
        assert(pPatternFile != 0);
        
        unsigned int NumberOfBytes = static_cast<unsigned int>(strlen(pPatternFile) * sizeof(char));
        const void*  pData         = static_cast<const void*>(pPatternFile);
        
        Hash = Base::CRC32(pData, NumberOfBytes);
        
        assert(Hash != 0);
        
        if (m_MarkerByHashs.find(Hash) != m_MarkerByHashs.end())
        {
            return CMarkerPtr(m_MarkerByHashs.at(Hash));
        }
        
		PathToPattern = g_PathToAssets + pPatternFile;
        
        // -----------------------------------------------------------------------------
        // If hash is not existing create a new marker
        // -----------------------------------------------------------------------------
        CMarkers::CPtr MarkerPtr;
        ARPattHandle*  pTemporaryPatternHandle = 0;
        int            TemporaryPatternID      = 0;
        
        try
        {
            // -----------------------------------------------------------------------------
            // Create internal marker
            // -----------------------------------------------------------------------------
            MarkerPtr = m_Markers.Allocate();
            
            CInternMarker& rMarker = *MarkerPtr;
            
            // -----------------------------------------------------------------------------
            // Setup basic marker informations and after that create handle
            // -----------------------------------------------------------------------------
            rMarker.m_UserID       = _rDescription.m_UserID;
            rMarker.m_WidthInMeter = _rDescription.m_WidthInMeter;
            rMarker.m_IsRegistered = false;
            
            pTemporaryPatternHandle = arPattCreateHandle();
            
            assert(pTemporaryPatternHandle != 0);

#ifdef __APPLE__
            TemporaryPatternID = arPattLoad(pTemporaryPatternHandle, PathToPattern.c_str());
#else
            TemporaryPatternID = LoadPattern(pTemporaryPatternHandle, PathToPattern.c_str());
#endif  
            
            assert(TemporaryPatternID >= 0);
            
            rMarker.m_pHandle = pTemporaryPatternHandle;
            rMarker.m_ID      = static_cast<unsigned int>(TemporaryPatternID);
            
            // -----------------------------------------------------------------------------
            // Set hash to map to reuse this kind of marker
            // -----------------------------------------------------------------------------
            m_MarkerByHashs[Hash]         = &rMarker;
            m_MarkerByIDs  [rMarker.m_ID] = &rMarker;
        }
        catch (...)
        {
            if (pTemporaryPatternHandle) arPattDeleteHandle(pTemporaryPatternHandle);

            BASE_THROWM("Undefined error creating marker in marker manager.");
        }
        
        return CMarkerPtr(MarkerPtr);
    }
    
    // -----------------------------------------------------------------------------
    
    CMarkerPtr CMarkerManager::GetMarkerByID(unsigned int _ID)
    {
        return CMarkerPtr(m_MarkerByIDs.at(_ID));
    }
} // namespace

namespace MR
{
namespace MarkerManager
{
    void OnStart()
    {
        CMarkerManager::GetInstance().OnStart();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnExit()
    {
        CMarkerManager::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    void Clear()
    {
        CMarkerManager::GetInstance().Clear();
    }
    
    // -----------------------------------------------------------------------------
    
    CMarkerPtr CreateMarker(const SMarkerDescription& _rDescription)
    {
        return CMarkerManager::GetInstance().CreateMarker(_rDescription);
    }
    
    // -----------------------------------------------------------------------------
    
    CMarkerPtr GetMarkerByID(unsigned int _ID)
    {
        return CMarkerManager::GetInstance().GetMarkerByID(_ID);
    }
} // namespace MarkerManager
} // namespace MR