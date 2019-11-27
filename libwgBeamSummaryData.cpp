// system includes
#include <string>

// ROOT includes
#include "TFile.h"
#include "TTree.h"
#include "TError.h"

// user includes
#include "wgConst.hpp"
#include "wgErrorCodes.hpp"
#include "wgFileSystemTools.hpp"
#include "wgGetTree.hpp"
#include "wgLogger.hpp"
#include "wgMakeHist.hpp"
#include "wgTopology.hpp"
using namespace wagasci_tools;

int wgBeamSummaryData(const char * x_input_file_name,
                      const char * x_topology_source,
                      const char * x_output_dir,
                      const bool overwrite,
                      const unsigned dif) {

  /////////////////////////////////////////////////////////////////////////////
  //                          arguments sanity check                         //
  /////////////////////////////////////////////////////////////////////////////
  
  std::string input_file_name(x_input_file_name);
  std::string topology_source(x_topology_source);
  std::string output_dir(x_output_dir);
  std::string output_file_name(input_file_name);

  if(input_file_name.empty() || !check_exist::root_file(input_file_name)) {
    Log.eWrite("[wgBeamSummaryData] Input file not found : " + input_file_name);
    return ERR_EMPTY_INPUT_FILE;
  }
  if (!wagasci_tools::check_exist::directory(output_dir)) {
    wagasci_tools::make::directory(output_dir);
  }
  if (dif > NDIFS) {
    Log.eWrite("[wgBeamSummaryData] DIF number is not valid : " +
               std::to_string(dif));
//    return ERR_WRONG_DIF;
      return ERR_WRONG_MODE;
  }

  Log.Write("[wgBeamSummaryData] *****  INPUT FILE         : " + input_file_name);
  Log.Write("[wgBeamSummaryData] *****  OUTPUT FILE        : " + output_file_name);
  Log.Write("[wgBeamSummaryData] *****  OUTPUT DIRECTORY   : " + output_dir);

  gErrorIgnoreLevel = kError;
  gROOT->SetBatch(kTRUE);

  /////////////////////////////////////////////////////////////////////////////
  //                                 Topology                                //
  /////////////////////////////////////////////////////////////////////////////

  std::unique_ptr<Topology> topol;
  try {
    nlohmann::json json = nlohmann::json::parse(topology_source);
    topol = boost::make_unique<Topology>(topology_source,
                                         TopologySourceType::json_string);
  } catch (...) {
    try {
      topol.reset(new Topology(topology_source, TopologySourceType::xml_file));
    } catch (const std::exception& except) {
      Log.eWrite("Topology string (" + topology_source + ") is not a valid"
                 "JSON string or a path to a Pyrame XML config file : "
                 + except.what());
      return ERR_TOPOLOGY;
    }
  }
  unsigned n_chips = topol->dif_map[dif].size();

  /////////////////////////////////////////////////////////////////////////////
  //                           Open tree.root file                           //
  /////////////////////////////////////////////////////////////////////////////
  
  Raw_t rd(n_chips);
  
  try {
    wgGetTree wg_tree(input_file_name, rd, dif); 

    /////////////////////////////////////////////////////////////////////////////
    //                         Get acquisition run info                        //
    /////////////////////////////////////////////////////////////////////////////
  
    int start_time =  wg_tree.GetStartTime();
    int stop_time  =  wg_tree.GetStartTime();

    int max_spill = wg_tree.tree->GetMaximum("spill_count");
    int min_spill = wg_tree.tree->GetMinimum("spill_count");
    spill_count = new TParameter<int>("spill_count",
                                      std::abs(max_spill - min_spill));
    Int_t n_events = wg_tree.tree->GetEntries();

    /////////////////////////////////////////////////////////////////////////////
    //                                Event loop                               //
    /////////////////////////////////////////////////////////////////////////////
  
    for (Int_t ievent = 0; ievent < n_events; ++ievent) {

      if (ievent % 1000 == 0)
        Log.Write("[wgMakeHist] Event number = " + std::to_string(ievent) +
                  " / " + std::to_string(n_events));
      // Read one event
      wg_tree.tree->GetEntry(ievent);

      ///////////////////////////////////////////////////////////////////////////////
      //                           DO YOUR ANALYSIS HERE                           //
      ///////////////////////////////////////////////////////////////////////////////


      
    }
  } catch (const std::exception& except) {
    Log.eWrite("[wgBeamSummaryData] failed to get the TTree from file : " +
               std::string(except.what()));
    return ERR_FAILED_OPEN_TREE_FILE;
  }

  return WG_SUCCESS;
}

#if 0
void BSD_INFO(
    string inputDirName,string outputDirName,
    int t2krun,int mrrun,int smrrun,int ssmrrun,
    string bsd_version){

  bool normal_cond;
  bool beamtrg;
  bool mucenter;
  bool muct;
  bool GoodSpillv01();
  bool GoodSpill();
  bool HornOff();
  bool Horn320();
  bool v01    ;
  bool hornoff;
  bool horn320;


  bool   isRHC     = false;
  int    count_good_spill = 0;
  double count_good_pot   = 0.;
  int    Nspill_nu     = 0;
  int    Nspill_antinu = 0;
  double Npot_nu       = 0.;
  double Npot_antinu   = 0.;

  string bsdfilename;
  if(smrrun==-1){
    bsdfilename = Form("/Users/ngocha/data_beam_wg/t2krun10/merge_bsd_run083_p06.root");
    //bsdfilename = Form("%s/%s/t2krun%d/merge_bsd_run%03d_%s.root",inputDirName.c_str(), bsd_version.c_str(), t2krun, mrrun, bsd_version.c_str());
  }
  else{
    bsdfilename = Form("/Users/ngocha/data_beam_wg/t2krun10/merge_bsd_run083_p06.root");
    //bsdfilename = Form("%s/%s/t2krun%d/bsd_run%03d%04d_%02d%s.root",inputDirName.c_str(),bsd_version.c_str(),t2krun,mrrun,smrrun,ssmrrun,bsd_version.c_str());
  }


  BSD_t* bsd_t = new BSD_t();
  if(!bsd_t->OpenBsdFile(bsdfilename)) return;
  if(bsd_version!=bsd_t->version) return;

  int nevt = bsd_t->bsd->GetEntries();

  string outtxtfilename  = Form("%s/spill_bsd_t2krun%d_mrrun%03d.txt"
                                ,outputDirName.c_str(),t2krun,mrrun);
  string outtxtfilename1 = Form("%s/spill_bsd_t2krun%d_mrrun%03d_fhc.txt"
                                ,outputDirName.c_str(),t2krun,mrrun);
  string outtxtfilename2 = Form("%s/spill_bsd_t2krun%d_mrrun%03d_rhc.txt"
                                ,outputDirName.c_str(),t2krun,mrrun);
  ofstream wfile (outtxtfilename .c_str());
  ofstream wfile1(outtxtfilename1.c_str());
  ofstream wfile2(outtxtfilename2.c_str());

  for(int ievt = 0; ievt < nevt; ievt++){
    if(ievt%100000==0){ cout<<" "<< ievt << flush; }
    bsd_t->bsd->GetEntry(ievt);

    bool ok = false;
    if(bsd_t->version=="v01"){
      if(bsd_t->good_spill_flag==1){
        ok = true;
        isRHC = false;
      }
      else if(bsd_t->good_spill_flag==-1){
        ok = true;
        isRHC = true;
      }
      else{ ok = false; }
    }
    else if(bsd_t->version=="p06"){
      double mucenter = sqrt(bsd_t->mumon[2]*bsd_t->mumon[2]+bsd_t->mumon[4]*bsd_t->mumon[4]);
      if     (bsd_t->spill_flag!=1   ) { ok = false; }
      else if(bsd_t->run_type  !=1   ) { ok = false; }
      else if(bsd_t->ct_np[4][0]<1e11) { ok = false; }
      else if(mucenter>10            ) { ok = false; }
      else{
        if(fabs(bsd_t->hct[0][0]-250.)<5.&&
           fabs(bsd_t->hct[1][0]-250.)<5.&&
           fabs(bsd_t->hct[2][0]-250.)<5.)
        {
          ok = true;
          isRHC = false;
        }
        else if(fabs(bsd_t->hct[0][0]+250.)<5.&&
                fabs(bsd_t->hct[1][0]+250.)<5.&&
                fabs(bsd_t->hct[2][0]+250.)<5.)
        {
          ok = true;
          isRHC = true;
        }
        else{
          ok = false;
        }
      }
    }

    if(ok){
      if(isRHC){
        Nspill_antinu++;
        Npot_antinu+=bsd_t->ct_np[4][0];
        wfile2
            << bsd_t->spillnum   << " "
            << isRHC             << " "
            << bsd_t->trg_sec[0] << " "
            << Npot_antinu       << endl;
      }
      else{
        Nspill_nu++;
        Npot_nu+=bsd_t->ct_np[4][0];
        wfile1
            << bsd_t->spillnum   << " "
            << isRHC             << " "
            << bsd_t->trg_sec[0] << " "
            << Npot_nu       << endl;
      }
      count_good_spill ++;
      count_good_pot   += bsd_t->ct_np[4][0];
      wfile
          << bsd_t->spillnum   << " "
          << isRHC             << " "
          << bsd_t->trg_sec[0] << " "
          //<< count_good_pot    << endl;
          << Form("%6.5e",bsd_t->ct_np[4][0]) << endl;
    }
  }

  wfile .close();
  wfile1.close();
  wfile2.close();

  cout<<endl;
  cout << "=============================================" << endl;
  cout << " Beam Data Summary: T2KRUN" << t2krun << ", MRRUN" << mrrun << endl;
  cout << " Total Spill: "  << count_good_spill
       << ", Total POT: "   << count_good_pot
       << endl;
  cout << "  FHC Spill: "  << Nspill_nu
       << ", POT: "        << Npot_nu
       << endl;
  cout << "  RHC Spill: "  << Nspill_antinu
       << ", POT: "        << Npot_antinu
       << endl;
  cout << "=============================================" << endl;
  cout<<endl;
}

void get_time(int t, int& yea, int& mon, int& mday){
  time_t aclock = t;
  struct tm *newtime;
  newtime = localtime(&aclock);
  mday = newtime -> tm_mday;
  mon  = newtime -> tm_mon+1;
  yea  = newtime -> tm_year+1900;
}
#endif
