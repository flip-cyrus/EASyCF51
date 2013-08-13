//---------------------------------------------------------------------------

#include <vcl.h>
#include <stdio.h>
#pragma hdrstop

#include "ProgrammerConfig.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TProgConfig *ProgConfig;

AnsiString workfilename;
AnsiString scriptfile;

//---------------------------------------------------------------------------
__fastcall TProgConfig::TProgConfig(TComponent* Owner)
        : TForm(Owner)
{
}
//---------------------------------------------------------------------------


void __fastcall TProgConfig::BitBtn1Click(TObject *Sender)
{
  if (e_algorithm->Text == "")
    return;
  FILE *f_script;
//  AnsiString scriptfile;
  if (scriptfile == "")
  {
    AnsiString scriptfilename = "prog.cfg";
    scriptfile = ExtractFilePath(_argv[0]) + scriptfilename;
  }
  try
  {
    f_script = fopen(scriptfile.c_str(), "w");
    if (cb_Reset->Checked)
      fprintf(f_script,"RE \n");
    fprintf(f_script,"CM %s 0\n", e_algorithm->Text);
    if (cb_Erase->Checked)
      fprintf(f_script,"EM \n");
    if (cb_blankcheck->Checked)
      fprintf(f_script,"BM \n");
    if (rb_default->Checked)
      fprintf(f_script,"SS %s\n", workfilename);
    else
      fprintf(f_script,"SS %s\n", e_srecord->Text);
    if (cb_Program->Checked)
      fprintf(f_script,"PM \n");
    if (cb_Verify->Checked)
      fprintf(f_script,"VM \n");
  }
  catch( ... )
  {
  }
  fclose(f_script);
  ProgConfig->Close();
}
//---------------------------------------------------------------------------


void __fastcall TProgConfig::btn_CFPBrowseClick(TObject *Sender)
{
  FO_Algorithm->Title = "Select Programming Algorithm";
  if(FO_Algorithm->Execute()) {
    e_algorithm->Text = FO_Algorithm->FileName;
  }
}
//---------------------------------------------------------------------------


void __fastcall TProgConfig::BitBtn2Click(TObject *Sender)
{
  ProgConfig->Close();        
}
//---------------------------------------------------------------------------

