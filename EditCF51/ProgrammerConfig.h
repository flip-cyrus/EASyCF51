//---------------------------------------------------------------------------

#ifndef ProgrammerConfigH
#define ProgrammerConfigH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Buttons.hpp>
#include <Dialogs.hpp>
//---------------------------------------------------------------------------
class TProgConfig : public TForm
{
__published:	// IDE-managed Components
        TCheckBox *cb_Reset;
        TCheckBox *cb_Erase;
        TCheckBox *cb_blankcheck;
        TCheckBox *cb_Program;
        TCheckBox *cb_Verify;
        TEdit *e_algorithm;
        TLabel *Label1;
        TOpenDialog *FO_Algorithm;
        TOpenDialog *OpenDialog1;
        TButton *btn_CFPBrowse;
        TRadioButton *rb_default;
        TLabel *Label2;
        TRadioButton *rb_custom;
        TEdit *e_srecord;
        TButton *btn_S19Browse;
        TBitBtn *BitBtn1;
        TBitBtn *BitBtn2;
        TLabel *Label3;
        void __fastcall BitBtn1Click(TObject *Sender);
        void __fastcall btn_CFPBrowseClick(TObject *Sender);
        void __fastcall BitBtn2Click(TObject *Sender);
private:	// User declarations
public:		// User declarations
        AnsiString s_record;
        __fastcall TProgConfig(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TProgConfig *ProgConfig;
//---------------------------------------------------------------------------
#endif
