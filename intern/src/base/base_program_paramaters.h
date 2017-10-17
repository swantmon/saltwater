
#pragma once

namespace IO
{
    class CProgramParameters
    {
    public:

        static CProgramParameters& GetInstance();

    public:

        template<class T>
        void AddParameter(const std::string& _rOption, const T& _rParamater);

    public:

        template<class T>
        const T& GetParameter(const std::string& _rOption);

        bool HasParameter(const std::string& _rOption);

    private:

        CProgramParameters();
        ~CProgramParameters();
    };
} // namespace IO

namespace IO
{
    CProgramParameters& CProgramParameters::GetInstance()
    {
        static CProgramParameters s_Sinstance;

        return s_Sinstance;
    }
} // namespace IO

namespace IO
{
    CProgramParameters::CProgramParameters()
    {

    }

    // -----------------------------------------------------------------------------

    CProgramParameters::~CProgramParameters()
    {

    }

    // -----------------------------------------------------------------------------

    template<class T>
    void CProgramParameters::AddParameter(const std::string& _rOption, const T& _rParamater)
    {

    }

    // -----------------------------------------------------------------------------

    template<class T>
    const T& CProgramParameters::GetParameter(const std::string& _rOption)
    {

    }

    // -----------------------------------------------------------------------------

    bool CProgramParameters::HasParameter(const std::string& _rOption)
    {
        return false;
    }
} // namespace IO