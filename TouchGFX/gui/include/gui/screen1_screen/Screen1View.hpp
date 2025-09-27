#ifndef SCREEN1VIEW_HPP
#define SCREEN1VIEW_HPP

#include <gui_generated/screen1_screen/Screen1ViewBase.hpp>
#include <gui/screen1_screen/Screen1Presenter.hpp>

class Screen1View : public Screen1ViewBase
{
public:
    Screen1View();
    virtual ~Screen1View() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
		
		//系统回调
		virtual void handleGestureEvent(const GestureEvent& event);
		virtual void handleTickEvent();
		
		//范围修改
    virtual void function50us();
    virtual void function100us();
    virtual void function200us();
    virtual void function32();
    virtual void function64();
    virtual void function128();
		
		//加减偏置偏置
    virtual void functionUp();
    virtual void functionDown();
		
		//修改波形
    virtual void functionSin();
    virtual void functionTriangular();
    virtual void functionSquare();
    virtual void functionSawtooth();
		
		//调整幅值、频率
    virtual void functionFreqChange(int value);
    virtual void functionAmpChange(int value);
		virtual void functionBiasChange(int value);
		
		//启动暂停
		virtual void functionToggleButtonFFT();
    virtual void functionToggleButtonScan();
		
		//显示波形调整
		virtual void functionCompressVertical();
    virtual void functionExpandVertical();
    virtual void functionCompressHorizontal();
    virtual void functionExpandHorizontal();
		
		
		
protected:
		
};

#endif // SCREEN1VIEW_HPP
