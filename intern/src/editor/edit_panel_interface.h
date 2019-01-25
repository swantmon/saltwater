
#pragma once

namespace Edit
{
namespace GUI
{
    class IPanel
    {
    public:

        inline void Show(bool _Flag = true);

        inline void Hide();

        inline bool IsVisible() const;

        inline virtual void Render() = 0;

        inline virtual const char* GetName() = 0;

    protected:

        bool m_IsVisible = true;
    };
} // namespace GUI
} // namespace Edit

namespace Edit
{
namespace GUI
{
    inline void IPanel::Show(bool _Flag)
    {
        m_IsVisible = _Flag;
    }

    // -----------------------------------------------------------------------------

    inline void IPanel::Hide()
    {
        m_IsVisible = false;
    }

    // -----------------------------------------------------------------------------

    inline bool IPanel::IsVisible() const
    {
        return m_IsVisible;
    }
} // namespace GUI
} // namespace Edit