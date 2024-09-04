// Copyright 2019-2020 CERN and copyright holders of ALICE O2.
// See https://alice-o2.web.cern.ch/copyright for details of the copyright holders.
// All rights not expressly granted are reserved.
//
// This software is distributed under the terms of the GNU General Public
// License v3 (GPL Version 3), copied verbatim in the file "COPYING".
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

/// \file   MonitorFilterBit.cxx
/// \author Andrea Rossi <andrea.rossi@cern.ch>
///
/// \brief Task performing basic checks on filter-bit selections.
///

#include "Framework/runDataProcessing.h"
#include "Framework/AnalysisTask.h"
#include "Common/DataModel/EventSelection.h"
#include "Common/DataModel/TrackSelectionTables.h"
#include "Common/Core/RecoDecay.h"
#include "Framework/runDataProcessing.h"
#include "Framework/ASoAHelpers.h"
#include "Framework/ASoA.h"

using namespace o2;
using namespace o2::framework;
using namespace o2::aod::track;
using namespace o2::aod::mctracklabel;
//using namespace o2::aod::track::trackextension;
using namespace o2::framework::expressions;

namespace o2::aod {

  namespace trackPairForEff {
    DECLARE_SOA_COLUMN(PtTPCtr, ptTPCtr, float);
    DECLARE_SOA_COLUMN(EtaTPCtr, etaTPCtr, float);
    DECLARE_SOA_COLUMN(PhiTPCtr, phiTPCtr, float);
    DECLARE_SOA_COLUMN(PtITStr, ptITStr, float);
    DECLARE_SOA_COLUMN(EtaITStr, etaITStr, float);
    DECLARE_SOA_COLUMN(PhiITStr, phiITStr, float);
    DECLARE_SOA_COLUMN(NClustITS, nClustITS, uint8_t);
    DECLARE_SOA_COLUMN(NClustTPC, nClustTPC, int16_t);
    DECLARE_SOA_COLUMN(McPtIfisSamePart, mcPtIfisSamePart, float);
    DECLARE_SOA_COLUMN(PairType, pairType, uint8_t);
  }
  DECLARE_SOA_TABLE(TrackPairForEffPP,"AOD","TRACKPAIREFFPP",
            trackPairForEff::PtTPCtr,trackPairForEff::EtaTPCtr,trackPairForEff::PhiTPCtr,
            trackPairForEff::PtITStr,trackPairForEff::EtaITStr,trackPairForEff::PhiITStr,
            trackPairForEff::NClustITS,trackPairForEff::NClustTPC,trackPairForEff::McPtIfisSamePart,o2::soa::Marker<1>);
  DECLARE_SOA_TABLE(TrackPairForEffNN,"AOD","TRACKPAIREFFNN",
            trackPairForEff::PtTPCtr,trackPairForEff::EtaTPCtr,trackPairForEff::PhiTPCtr,
            trackPairForEff::PtITStr,trackPairForEff::EtaITStr,trackPairForEff::PhiITStr,
            trackPairForEff::NClustITS,trackPairForEff::NClustTPC,trackPairForEff::McPtIfisSamePart,o2::soa::Marker<2>);
  DECLARE_SOA_TABLE(TrackPairForEffPN,"AOD","TRACKPAIREFFPN",
            trackPairForEff::PtTPCtr,trackPairForEff::EtaTPCtr,trackPairForEff::PhiTPCtr,
            trackPairForEff::PtITStr,trackPairForEff::EtaITStr,trackPairForEff::PhiITStr,
            trackPairForEff::NClustITS,trackPairForEff::NClustTPC,trackPairForEff::McPtIfisSamePart,o2::soa::Marker<3>);
  DECLARE_SOA_TABLE(TrackPairForEffNP,"AOD","TRACKPAIREFFNP",
            trackPairForEff::PtTPCtr,trackPairForEff::EtaTPCtr,trackPairForEff::PhiTPCtr,
            trackPairForEff::PtITStr,trackPairForEff::EtaITStr,trackPairForEff::PhiITStr,
            trackPairForEff::NClustITS,trackPairForEff::NClustTPC,trackPairForEff::McPtIfisSamePart,o2::soa::Marker<4>);
}

struct CheckFilterBit
{

  Produces<aod::TrackPairForEffPP> trackPairForEffTablePP;
  Produces<aod::TrackPairForEffNN> trackPairForEffTableNN;
  Produces<aod::TrackPairForEffNP> trackPairForEffTableNP;
  Produces<aod::TrackPairForEffPN> trackPairForEffTablePN;

  // Binning
  ConfigurableAxis binsPt{"binsPt", {VARIABLE_WIDTH, 0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0, 1.1, 1.2, 1.3, 1.4, 1.5, 2.0, 5.0, 10.0, 20.0, 50.0}, ""};
  ConfigurableAxis binsEta{"binsEta", {30, -1.5, 1.5}, ""};
  Configurable<float> zVtxCut{"zVtxCut", 10., "Primary Vtx z cut"};
  ConfigurableAxis binsPhi{"binsPhi", {180, 0., 2 * M_PI}, "Phi binning"};
  ConfigurableAxis binsTPCITSmatching{"binsTPCITSmatching", {2, 0.5, 2.5}, "ITSTPCmatching"};
  ConfigurableAxis binsNclustTPC{"binsNclustTPC", {VARIABLE_WIDTH, -0.5, 0.5, 10, 50, 60, 70, 80, 90, 100, 105, 110, 115, 120, 125, 130, 135, 140, 145, 150, 155, 160}, ""};
  // ConfigurableAxis binsEta{"binsEta", {30, -1.5, 1.5}, ""};
  ConfigurableAxis binsTpcNCls{"binsTpcNCls",{153,0,153}, "TPC N Cls binning"};
  ConfigurableAxis binsTpcNClsCrossRows{"binsTpcNClsCrossRows",{153,0,153}, "TPC NCls cross rows binning"}; 
  ConfigurableAxis binsTpcNClsCrossRowsOvrFindCls{"binsTpcNClsCrossRowsOvrFindCls",{100,0.5,1.5}, "Rat TPC cross rows over findable binning"}; 
  ConfigurableAxis binsTpcChi2NCls{"binsTpcChi2NCls",{50,0,5},"TPC Chi2 NCls binning"};
  ConfigurableAxis binsItsNCls{"binsItsNCls",{10,0,10},"ITS N Cls binning"};
  ConfigurableAxis binsItsChi2NCls{"binsItsChi2NCls",{60,0,30},"ITS Chi2 NCls binning"};
  ConfigurableAxis binsTrkDcaXY{"binsTrkDcaXY",{500,-0.5,0.5}, "Trk DCA XY binning"};
  ConfigurableAxis binsTrkDcaZ{"binsTrkDcaZ",{500,-0.5,0.5},"Trk DCA Z binning"};

  HistogramRegistry histos;
  Int_t ncollisionCounter = 0;
  float fzero=0.;
  using Tracksextension=soa::Join<aod::Tracks, aod::TracksExtra, aod::TrackSelection, aod::TrackSelectionExtension,aod::TracksDCA>;
  using TracksextensionMC= soa::Join<Tracksextension, aod::McTrackLabels>;
  // Partition<Tracksextension> positiveTPConlyTracks=o2::aod::track::signed1Pt >fzero && ((o2::aod::track::detectorMap / o2::aod::track::TPC)/2.-(o2::aod::track::detectorMap / o2::aod::track::TPC)/2) <0.49;// && (o2::aod::track::detectorMap & o2::aod::track::ITS) ==0;
  // Partition<Tracksextension> negativeTPConlyTracks=o2::aod::track::signed1Pt <fzero && (o2::aod::track::detectorMap & o2::aod::track::TPC) ==o2::aod::track::TPC && (o2::aod::track::detectorMap & o2::aod::track::ITS) ==0;
  // Partition<Tracksextension> positiveITSonlyTracks=o2::aod::track::signed1Pt >fzero && (o2::aod::track::detectorMap & o2::aod::track::TPC) ==0 && (o2::aod::track::detectorMap & o2::aod::track::ITS) ==o2::aod::track::ITS && o2::aod::track::passedITSNCls==true;// && o2::aod::track::itsNCls==7;
  // Partition<Tracksextension> negativeITSonlyTracks=o2::aod::track::signed1Pt <fzero && (o2::aod::track::detectorMap & o2::aod::track::TPC)  ==0 &&(o2::aod::track::detectorMap & o2::aod::track::ITS) ==o2::aod::track::ITS && o2::aod::track::passedITSNCls==true;// && o2::aod::track::itsNCls==7;

  SliceCache cache;
  Partition<Tracksextension> positiveTPConlyTracks=o2::aod::track::signed1Pt >fzero && o2::aod::track::tpcNClsFindable>(uint8_t)0 && o2::aod::track::itsChi2NCl < (float_t)0;//&& (o2::aod::track::detectorMap & o2::aod::track::TPC) ==o2::aod::track::TPC && (o2::aod::track::detectorMap & o2::aod::track::ITS) ==0;
  Partition<Tracksextension> negativeTPConlyTracks=o2::aod::track::signed1Pt <fzero && o2::aod::track::tpcNClsFindable>(uint8_t)0 && o2::aod::track::itsChi2NCl < (float_t)0;//&& (o2::aod::track::detectorMap & o2::aod::track::TPC) ==o2::aod::track::TPC && (o2::aod::track::detectorMap & o2::aod::track::ITS) ==0;
  Partition<Tracksextension> positiveITSonlyTracks=o2::aod::track::signed1Pt >fzero && o2::aod::track::tpcChi2NCl<(float_t)0 && o2::aod::track::itsChi2NCl > (float_t)0;// && (o2::aod::track::detectorMap & o2::aod::track::TPC) ==0 && (o2::aod::track::detectorMap & o2::aod::track::ITS) ==o2::aod::track::ITS && o2::aod::track::passedITSNCls==true;// && o2::aod::track::itsNCls==7;
  Partition<Tracksextension> negativeITSonlyTracks=o2::aod::track::signed1Pt <fzero && o2::aod::track::tpcChi2NCl<(float_t)0 && o2::aod::track::itsChi2NCl > (float_t)0;// && (o2::aod::track::detectorMap & o2::aod::track::TPC)  ==0 &&(o2::aod::track::detectorMap & o2::aod::track::ITS) ==o2::aod::track::ITS && o2::aod::track::passedITSNCls==true;// && o2::aod::track::itsNCls==7;


  Partition<TracksextensionMC> positiveTPConlyTracksMC=o2::aod::track::signed1Pt >fzero && o2::aod::track::tpcNClsFindable>(uint8_t)0 && o2::aod::track::itsChi2NCl < (float_t)0;//&& (o2::aod::track::detectorMap & o2::aod::track::TPC) ==o2::aod::track::TPC && (o2::aod::track::detectorMap & o2::aod::track::ITS) ==0;
  Partition<TracksextensionMC> negativeTPConlyTracksMC=o2::aod::track::signed1Pt <fzero && o2::aod::track::tpcNClsFindable>(uint8_t)0 && o2::aod::track::itsChi2NCl < (float_t)0;//&& (o2::aod::track::detectorMap & o2::aod::track::TPC) ==o2::aod::track::TPC && (o2::aod::track::detectorMap & o2::aod::track::ITS) ==0;
  Partition<TracksextensionMC> positiveITSonlyTracksMC=o2::aod::track::signed1Pt >fzero && o2::aod::track::tpcChi2NCl<(float_t)0 && o2::aod::track::itsChi2NCl > (float_t)0;// && (o2::aod::track::detectorMap & o2::aod::track::TPC) ==0 && (o2::aod::track::detectorMap & o2::aod::track::ITS) ==o2::aod::track::ITS && o2::aod::track::passedITSNCls==true;// && o2::aod::track::itsNCls==7;
  Partition<TracksextensionMC> negativeITSonlyTracksMC=o2::aod::track::signed1Pt <fzero && o2::aod::track::tpcChi2NCl<(float_t)0 && o2::aod::track::itsChi2NCl > (float_t)0;// && (o2::aod::track::detectorMap & o2::aod::track::TPC)  ==0 &&(o2::aod::track::detectorMap & o2::aod::track::ITS) ==o2::aod::track::ITS && o2::aod::track::passedITSNCls==true;// && o2::aod::track::itsNCls==7;

  //  Filter positiveTPConlyTracksFilter=o2::aod::track::pt > 1;//o2::aod::track::itsNCls==7;

  void init(InitContext const&)
  {

    const AxisSpec axisPt{binsPt, "#it{p}_{T} (GeV/c)"};
    const AxisSpec axisEta{binsEta, "#it{#eta}"};
    const AxisSpec axisPhi{binsPhi, "#it{#varphi}"};
    const AxisSpec axisNclustTPC{binsNclustTPC, "NclustTPC"};
    const AxisSpec axisTPCITSmatching{binsNclustTPC, "NclustTPC"};
    const AxisSpec axisTpcNCls{binsTpcNCls,"TPC N clusters"};
    const AxisSpec axisTpcNClsCrossRows{binsTpcNClsCrossRows,"TPC N Cluster Crossed Rows"};
    const AxisSpec axisTpcNClsCrossRowsOvrFindCls{binsTpcNClsCrossRowsOvrFindCls,"Ratio Findable Cls/Crossed Rows"};
    const AxisSpec axisTpcChi2NCls{binsTpcChi2NCls,"TPC #chi^2 NCls"}; 
    const AxisSpec axisItsNCls{binsItsNCls,"ITS N clusters"};
    const AxisSpec axisItsChi2NCls{binsItsChi2NCls,"ITS #chi^2 NCls"};
    const AxisSpec axisTrkDcaXY{binsTrkDcaXY,"DCA XY"};
    const AxisSpec axisTrkDcaZ{binsTrkDcaZ,"DCA Z"};

    histos.add("EventProp/histDataNEvents","Event counter, Data", kTH1D, {{1,0,1}});
    histos.add("EventProp/histRecoMcNEvents","Event counter, MC Reco", kTH1D, {{1,0,1}});
    histos.add("EventProp/histGenMcNEvents","Event counter, MC Gen", kTH1D, {{1,0,1}}); 
    histos.add("EventProp/histMCcollZ", "MC coll Z (cm); #it{z_{MCcoll}} (cm)", kTH1D, {{100, -20., 20.}});
    histos.add("EventProp/histRecoMCcollZ","Reco MC coll Z (cm); #it{z_{MCcoll}} (cm)", kTH1D, {{100, -20., 20.}});
    histos.add("EventProp/histDatacollZ", "Data coll Z (cm); #it{z_{MCcoll}} (cm)", kTH1D, {{100, -20., 20.}});
    histos.add("EventProp/histPtTrackNegCollID", "pt", kTH1D, {axisPt});

    histos.add("Tracks/Reco/histptAll", "pt", kTH1D, {axisPt});
    histos.add("Tracks/Reco/histpt3DAll", "All tracks;#it{p}_{T} (GeV/#it{c});#it{#eta};#it{#varphi}", kTH3D, {axisPt, axisEta, axisPhi});
    histos.add("Tracks/Reco/histpt3DFB0", "FB0 tracks;#it{p}_{T} (GeV/#it{c});#it{#eta};#it{#varphi}", kTH3D, {axisPt, axisEta, axisPhi});
    histos.add("Tracks/Reco/histpt3DFB1", "FB1 tracks;#it{p}_{T} (GeV/#it{c});#it{#eta};#it{#varphi}", kTH3D, {axisPt, axisEta, axisPhi});
    histos.add("Tracks/Reco/histpt3DFB2", "FB2 tracks;#it{p}_{T} (GeV/#it{c});#it{#eta};#it{#varphi}", kTH3D, {axisPt, axisEta, axisPhi});
    histos.add("Tracks/Reco/histpt3DFB3", "FB3 tracks;#it{p}_{T} (GeV/#it{c});#it{#eta};#it{#varphi}", kTH3D, {axisPt, axisEta, axisPhi});
    histos.add("Tracks/Reco/histpt3DFB4", "FB4 tracks;#it{p}_{T} (GeV/#it{c});#it{#eta};#it{#varphi}", kTH3D, {axisPt, axisEta, axisPhi});
    histos.add("Tracks/Reco/histpt3DFB5", "FB5 tracks;#it{p}_{T} (GeV/#it{c});#it{#eta};#it{#varphi}", kTH3D, {axisPt, axisEta, axisPhi});
    histos.add("Tracks/Reco/histpt3DITSonly", "ITSonly tracks;#it{p}_{T} (GeV/#it{c});#it{#eta};#it{#varphi}", kTH3D, {axisPt, axisEta, axisPhi});
    histos.add("Tracks/Reco/histpt3DTPConly", "TPConly tracks;#it{p}_{T} (GeV/#it{c});#it{#eta};#it{#varphi}", kTH3D, {axisPt, axisEta, axisPhi});

    histos.add("Tracks/Reco/histptGbNoDca", "Global track wo DCA cut;#it{p}_{T} (GeV/#it{c});#it{#eta};#it{#varphi}", kTH3D, {axisPt, axisEta, axisPhi});
    histos.add("Tracks/Reco/histGbTrkTpcNCls", "Global track wo DCA cut;#it{p}_{T} (GeV/#it{c});TPC NCls",kTH2D,{axisPt,axisTpcNCls});
    histos.add("Tracks/Reco/histGbTrkTpcNClsCrossRows", "Global track wo DCA cut;#it{p}_{T} (GeV/#it{c});TPC N CrossRows",kTH2D,{axisPt,axisTpcNClsCrossRows});
    histos.add("Tracks/Reco/histGbTrkTpcNClsCrossRowsOvrFindableCls", "Global track wo DCA cut;#it{p}_{T} (GeV/#it{c});Ratio N CrossRow/N Findable",kTH2D,{axisPt,axisTpcNClsCrossRowsOvrFindCls});
    histos.add("Tracks/Reco/histGbTrkTpcChi2NCls", "Global track wo DCA cut;#it{p}_{T} (GeV/#it{c});TPC Chi2 NCls",kTH2D,{axisPt,axisTpcChi2NCls});
    histos.add("Tracks/Reco/histGbTrkItsNCls", "Global track wo DCA cut;#it{p}_{T} (GeV/#it{c});ITS N Cls",kTH2D,{axisPt,axisItsNCls});
    histos.add("Tracks/Reco/histGbTrkItsChi2NCls", "Global track wo DCA cut;#it{p}_{T} (GeV/#it{c});ITS Chi2 NCls",kTH2D,{axisPt,axisItsChi2NCls});
    histos.add("Tracks/Reco/histGbTrkDcaXY", "Global track wo DCA cut;#it{p}_{T} (GeV/#it{c});DCA XY",kTH2D,{axisPt,axisTrkDcaXY});
    histos.add("Tracks/Reco/histGbTrkDcaZ", "Global track wo DCA cut;#it{p}_{T} (GeV/#it{c});DCA Z",kTH2D,{axisPt,axisTrkDcaZ});
      
    histos.add("Tracks/Reco/histptNewConfig", "New Config cut;#it{p}_{T} (GeV/#it{c});#it{#eta};#it{#varphi}", kTH3D, {axisPt, axisEta, axisPhi});
    histos.add("Tracks/Reco/histNewConfigTpcNCls", "New Config cut;#it{p}_{T} (GeV/#it{c});TPC NCls",kTH2D,{axisPt,axisTpcNCls});
    histos.add("Tracks/Reco/histNewConfigTpcNClsCrossRows", "New Config cut;#it{p}_{T} (GeV/#it{c});TPC N CrossRows",kTH2D,{axisPt,axisTpcNClsCrossRows});
    histos.add("Tracks/Reco/histNewConfigTpcNClsCrossRowsOvrFindableCls", "New Config cut;#it{p}_{T} (GeV/#it{c});Ratio N CrossRow/N Findable",kTH2D,{axisPt,axisTpcNClsCrossRowsOvrFindCls});
    histos.add("Tracks/Reco/histNewConfigTpcChi2NCls", "New Config cut;#it{p}_{T} (GeV/#it{c});TPC Chi2 NCls",kTH2D,{axisPt,axisTpcChi2NCls});
    histos.add("Tracks/Reco/histNewConfigDcaXY", "New Config cut;#it{p}_{T} (GeV/#it{c});DCA XY",kTH2D,{axisPt,axisTrkDcaXY});
    histos.add("Tracks/Reco/histNewConfigDcaZ", "New Config cut;#it{p}_{T} (GeV/#it{c});DCA Z",kTH2D,{axisPt,axisTrkDcaZ});

    histos.add("Tracks/MCgen/histMCgenpt", "pt", kTH1D, {axisPt});
    histos.add("Tracks/MCgen/histMCgen3dPhysPrimary", "MC Phys. Prim.;#it{p}_{T} (GeV/#it{c});#it{#eta};#it{#varphi}", kTH3D, {axisPt, axisEta, axisPhi});
    histos.add("Tracks/MCgen/histMCgen3dChargedProdRad1to15cm", "MC Prod Rad_xy 1 to 15 cm;#it{p}_{T} (GeV/#it{c});#it{#eta};#it{#varphi}", kTH3D, {axisPt, axisEta, axisPhi});
    histos.add("Tracks/MCgen/histMCgen3dChargedProdRad1mumto5mm", "MC Prod Rad_xy 1#mum to 5 mm ;#it{p}_{T} (GeV/#it{c});#it{#eta};#it{#varphi}", kTH3D, {axisPt, axisEta, axisPhi});
    histos.add("Tracks/MCgen/histMCgen3dChargedfromHFdecay", "MC Phys. Prim from HF decay ;#it{p}_{T} (GeV/#it{c});#it{#eta};#it{#varphi}", kTH3D, {axisPt, axisEta, axisPhi});
      
    histos.add("Tracks/RecoMC/histptNewConfig", "New Config cut;#it{p}_{T} (GeV/#it{c});#it{#eta};#it{#varphi}", kTH3D, {axisPt, axisEta, axisPhi});
    histos.add("Tracks/RecoMC/histNewConfigTpcNCls", "New Config cut;#it{p}_{T} (GeV/#it{c});TPC NCls",kTH2D,{axisPt,axisTpcNCls});
    histos.add("Tracks/RecoMC/histNewConfigTpcNClsCrossRows", "New Config cut;#it{p}_{T} (GeV/#it{c});TPC N CrossRows",kTH2D,{axisPt,axisTpcNClsCrossRows});
    histos.add("Tracks/RecoMC/histNewConfigTpcNClsCrossRowsOvrFindableCls", "New Config cut;#it{p}_{T} (GeV/#it{c});Ratio N CrossRow/N Findable",kTH2D,{axisPt,axisTpcNClsCrossRowsOvrFindCls});
    histos.add("Tracks/RecoMC/histNewConfigTpcChi2NCls", "New Config cut;#it{p}_{T} (GeV/#it{c});TPC Chi2 NCls",kTH2D,{axisPt,axisTpcChi2NCls});
    histos.add("Tracks/RecoMC/histNewConfigDcaXY", "New Config cut;#it{p}_{T} (GeV/#it{c});DCA XY",kTH2D,{axisPt,axisTrkDcaXY});
    histos.add("Tracks/RecoMC/histNewConfigDcaZ", "New Config cut;#it{p}_{T} (GeV/#it{c});DCA Z",kTH2D,{axisPt,axisTrkDcaZ});

    histos.add("Tracks/RecoMCPhysPrim/histptNewConfig", "New Config cut;#it{p}_{T} (GeV/#it{c});#it{#eta};#it{#varphi}", kTH3D, {axisPt, axisEta, axisPhi});
    histos.add("Tracks/RecoMCPhysPrim/histNewConfigTpcNCls", "New Config cut;#it{p}_{T} (GeV/#it{c});TPC NCls",kTH2D,{axisPt,axisTpcNCls});
    histos.add("Tracks/RecoMCPhysPrim/histNewConfigTpcNClsCrossRows", "New Config cut;#it{p}_{T} (GeV/#it{c});TPC N CrossRows",kTH2D,{axisPt,axisTpcNClsCrossRows});
    histos.add("Tracks/RecoMCPhysPrim/histNewConfigTpcNClsCrossRowsOvrFindableCls", "New Config cut;#it{p}_{T} (GeV/#it{c});Ratio N CrossRow/N Findable",kTH2D,{axisPt,axisTpcNClsCrossRowsOvrFindCls});
    histos.add("Tracks/RecoMCPhysPrim/histNewConfigTpcChi2NCls", "New Config cut;#it{p}_{T} (GeV/#it{c});TPC Chi2 NCls",kTH2D,{axisPt,axisTpcChi2NCls});
    histos.add("Tracks/RecoMCPhysPrim/histNewConfigDcaXY", "New Config cut;#it{p}_{T} (GeV/#it{c});DCA XY",kTH2D,{axisPt,axisTrkDcaXY});
    histos.add("Tracks/RecoMCPhysPrim/histNewConfigDcaZ", "New Config cut;#it{p}_{T} (GeV/#it{c});DCA Z",kTH2D,{axisPt,axisTrkDcaZ});

    histos.add("Tracks/RecoMCPhysPrimCollMatch/histpt", "pt;#it{p}_{T}^{MC} (GeV/#it{c})", kTH1D, {axisPt});
    histos.add("Tracks/RecoMCPhysPrimCollMatch/histptFB0", "FB0;#it{p}_{T} (GeV/#it{c});#it{#eta};#it{#varphi}", kTH3D, {axisPt, axisEta, axisPhi});
    histos.add("Tracks/RecoMCPhysPrimCollMatch/histptFB1", "FB1;#it{p}_{T} (GeV/#it{c});#it{#eta};#it{#varphi}", kTH3D, {axisPt, axisEta, axisPhi});
    histos.add("Tracks/RecoMCPhysPrimCollMatch/histptFB2", "FB2;#it{p}_{T} (GeV/#it{c});#it{#eta};#it{#varphi}", kTH3D, {axisPt, axisEta, axisPhi});
    histos.add("Tracks/RecoMCPhysPrimCollMatch/histptFB3", "FB3;#it{p}_{T} (GeV/#it{c});#it{#eta};#it{#varphi}", kTH3D, {axisPt, axisEta, axisPhi});
    histos.add("Tracks/RecoMCPhysPrimCollMatch/histptFB4", "FB4;#it{p}_{T} (GeV/#it{c});#it{#eta};#it{#varphi}", kTH3D, {axisPt, axisEta, axisPhi});
    histos.add("Tracks/RecoMCPhysPrimCollMatch/histptFB5", "FB5;#it{p}_{T} (GeV/#it{c});#it{#eta};#it{#varphi}", kTH3D, {axisPt, axisEta, axisPhi});
    histos.add("Tracks/RecoMCPhysPrimCollMatch/histptITSonly", "ITSonly;#it{p}_{T} (GeV/#it{c});#it{#eta};#it{#varphi}", kTH3D, {axisPt, axisEta, axisPhi});
    histos.add("Tracks/RecoMCPhysPrimCollMatch/histptTPConly", "TPConly;#it{p}_{T} (GeV/#it{c});#it{#eta};#it{#varphi}", kTH3D, {axisPt, axisEta, axisPhi});

    histos.add("Tracks/RecoMCPhysPrimCollMatch/histptMCFB0", "FB0;#it{p}_{T}^{MC} (GeV/#it{c});#it{#eta};#it{#varphi}", kTH3D, {axisPt, axisEta, axisPhi});
    histos.add("Tracks/RecoMCPhysPrimCollMatch/histptMCFB1", "FB1;#it{p}_{T}^{MC} (GeV/#it{c});#it{#eta};#it{#varphi}", kTH3D, {axisPt, axisEta, axisPhi});
    histos.add("Tracks/RecoMCPhysPrimCollMatch/histptMCFB2", "FB2;#it{p}_{T}^{MC} (GeV/#it{c});#it{#eta};#it{#varphi}", kTH3D, {axisPt, axisEta, axisPhi});
    histos.add("Tracks/RecoMCPhysPrimCollMatch/histptMCFB3", "FB3;#it{p}_{T}^{MC} (GeV/#it{c});#it{#eta};#it{#varphi}", kTH3D, {axisPt, axisEta, axisPhi});
    histos.add("Tracks/RecoMCPhysPrimCollMatch/histptMCFB4", "FB4;#it{p}_{T}^{MC} (GeV/#it{c});#it{#eta};#it{#varphi}", kTH3D, {axisPt, axisEta, axisPhi});
    histos.add("Tracks/RecoMCPhysPrimCollMatch/histptMCFB5", "FB5;#it{p}_{T}^{MC} (GeV/#it{c});#it{#eta};#it{#varphi}", kTH3D, {axisPt, axisEta, axisPhi});
    histos.add("Tracks/RecoMCPhysPrimCollMatch/histptMCITSonly", "ITSonly;#it{p}_{T}^{MC} (GeV/#it{c});#it{#eta};#it{#varphi}", kTH3D, {axisPt, axisEta, axisPhi});
    histos.add("Tracks/RecoMCPhysPrimCollMatch/histptMCTPConly", "TPConly;#it{p}_{T}^{MC} (GeV/#it{c});#it{#eta};#it{#varphi}", kTH3D, {axisPt, axisEta, axisPhi});
    histos.add("Tracks/RecoMCPhysPrimCollMatch/histptMCTPConlyWithClusters", "TPConlyWithClusters;#it{p}_{T}^{gen} (GeV/#it{c});#it{#eta};#it{#varphi};NclustTPC",  HistType::kTHnF, {axisPt, axisEta, axisPhi, axisNclustTPC});
    histos.add("Tracks/RecoMCPhysPrimCollMatch/histptTPConlyWithClusters", "TPConlyWithClusters;#it{p}_{T} (GeV/#it{c});#it{#eta};#it{#varphi};NclustTPC",  HistType::kTHnF, {axisPt, axisEta, axisPhi, axisNclustTPC});

    histos.add("Tracks/RecoMCPhysPrimCollMatch/histptGbNoDca", "Global track wo DCA cut;#it{p}_{T} (GeV/#it{c});#it{#eta};#it{#varphi}", kTH3D, {axisPt, axisEta, axisPhi});
    histos.add("Tracks/RecoMCPhysPrimCollMatch/histGbTrkTpcNCls", "Global track wo DCA cut;#it{p}_{T} (GeV/#it{c});TPC NCls",kTH2D,{axisPt,axisTpcNCls});
    histos.add("Tracks/RecoMCPhysPrimCollMatch/histGbTrkTpcNClsCrossRows", "Global track wo DCA cut;#it{p}_{T} (GeV/#it{c});TPC N CrossRows",kTH2D,{axisPt,axisTpcNClsCrossRows});
    histos.add("Tracks/RecoMCPhysPrimCollMatch/histGbTrkTpcNClsCrossRowsOvrFindableCls", "Global track wo DCA cut;#it{p}_{T} (GeV/#it{c});Ratio N CrossRow/N Findable",kTH2D,{axisPt,axisTpcNClsCrossRowsOvrFindCls});
    histos.add("Tracks/RecoMCPhysPrimCollMatch/histGbTrkTpcChi2NCls", "Global track wo DCA cut;#it{p}_{T} (GeV/#it{c});TPC Chi2 NCls",kTH2D,{axisPt,axisTpcChi2NCls});
    histos.add("Tracks/RecoMCPhysPrimCollMatch/histGbTrkItsNCls", "Global track wo DCA cut;#it{p}_{T} (GeV/#it{c});ITS N Cls",kTH2D,{axisPt,axisItsNCls});
    histos.add("Tracks/RecoMCPhysPrimCollMatch/histGbTrkItsChi2NCls", "Global track wo DCA cut;#it{p}_{T} (GeV/#it{c});ITS Chi2 NCls",kTH2D,{axisPt,axisItsChi2NCls});
    histos.add("Tracks/RecoMCPhysPrimCollMatch/histGbTrkDcaXY", "Global track wo DCA cut;#it{p}_{T} (GeV/#it{c});DCA XY",kTH2D,{axisPt,axisTrkDcaXY});
    histos.add("Tracks/RecoMCPhysPrimCollMatch/histGbTrkDcaZ", "Global track wo DCA cut;#it{p}_{T} (GeV/#it{c});DCA Z",kTH2D,{axisPt,axisTrkDcaZ});
      
      histos.add("Tracks/RecoMCPhysPrimCollMatch/histptNewConfig", "New Config cut;#it{p}_{T} (GeV/#it{c});#it{#eta};#it{#varphi}", kTH3D, {axisPt, axisEta, axisPhi});
      histos.add("Tracks/RecoMCPhysPrimCollMatch/histNewConfigTpcNCls", "New Config cut;#it{p}_{T} (GeV/#it{c});TPC NCls",kTH2D,{axisPt,axisTpcNCls});
      histos.add("Tracks/RecoMCPhysPrimCollMatch/histNewConfigTpcNClsCrossRows", "New Config cut;#it{p}_{T} (GeV/#it{c});TPC N CrossRows",kTH2D,{axisPt,axisTpcNClsCrossRows});
      histos.add("Tracks/RecoMCPhysPrimCollMatch/histNewConfigTpcNClsCrossRowsOvrFindableCls", "New Config cut;#it{p}_{T} (GeV/#it{c});Ratio N CrossRow/N Findable",kTH2D,{axisPt,axisTpcNClsCrossRowsOvrFindCls});
      histos.add("Tracks/RecoMCPhysPrimCollMatch/histNewConfigTpcChi2NCls", "New Config cut;#it{p}_{T} (GeV/#it{c});TPC Chi2 NCls",kTH2D,{axisPt,axisTpcChi2NCls});
      histos.add("Tracks/RecoMCPhysPrimCollMatch/histNewConfigDcaXY", "New Config cut;#it{p}_{T} (GeV/#it{c});DCA XY",kTH2D,{axisPt,axisTrkDcaXY});
      histos.add("Tracks/RecoMCPhysPrimCollMatch/histNewConfigDcaZ", "New Config cut;#it{p}_{T} (GeV/#it{c});DCA Z",kTH2D,{axisPt,axisTrkDcaZ});

    histos.add("Tracks/RecoMCRad1to15cmCollMatch/histptFB0", "FB0;#it{p}_{T} (GeV/#it{c});#it{#eta};#it{#varphi}", kTH3D, {axisPt, axisEta, axisPhi});
    histos.add("Tracks/RecoMCRad1to15cmCollMatch/histptFB1", "FB1;#it{p}_{T} (GeV/#it{c});#it{#eta};#it{#varphi}", kTH3D, {axisPt, axisEta, axisPhi});
    histos.add("Tracks/RecoMCRad1to15cmCollMatch/histptFB2", "FB2;#it{p}_{T} (GeV/#it{c});#it{#eta};#it{#varphi}", kTH3D, {axisPt, axisEta, axisPhi});
    histos.add("Tracks/RecoMCRad1to15cmCollMatch/histptFB3", "FB3;#it{p}_{T} (GeV/#it{c});#it{#eta};#it{#varphi}", kTH3D, {axisPt, axisEta, axisPhi});
    histos.add("Tracks/RecoMCRad1to15cmCollMatch/histptFB4", "FB4;#it{p}_{T} (GeV/#it{c});#it{#eta};#it{#varphi}", kTH3D, {axisPt, axisEta, axisPhi});
    histos.add("Tracks/RecoMCRad1to15cmCollMatch/histptFB5", "FB5;#it{p}_{T} (GeV/#it{c});#it{#eta};#it{#varphi}", kTH3D, {axisPt, axisEta, axisPhi});
    histos.add("Tracks/RecoMCRad1to15cmCollMatch/histptITSonly", "ITSonly;#it{p}_{T} (GeV/#it{c});#it{#eta};#it{#varphi}", kTH3D, {axisPt, axisEta, axisPhi});
    histos.add("Tracks/RecoMCRad1to15cmCollMatch/histptTPConly", "TPConly;#it{p}_{T} (GeV/#it{c});#it{#eta};#it{#varphi}", kTH3D, {axisPt, axisEta, axisPhi});

    histos.add("Tracks/RecoMCRad1mumto5mmCollMatch/histptFB0", "FB0;#it{p}_{T} (GeV/#it{c});#it{#eta};#it{#varphi}", kTH3D, {axisPt, axisEta, axisPhi});
    histos.add("Tracks/RecoMCRad1mumto5mmCollMatch/histptFB1", "FB1;#it{p}_{T} (GeV/#it{c});#it{#eta};#it{#varphi}", kTH3D, {axisPt, axisEta, axisPhi});
    histos.add("Tracks/RecoMCRad1mumto5mmCollMatch/histptFB2", "FB2;#it{p}_{T} (GeV/#it{c});#it{#eta};#it{#varphi}", kTH3D, {axisPt, axisEta, axisPhi});
    histos.add("Tracks/RecoMCRad1mumto5mmCollMatch/histptFB3", "FB3;#it{p}_{T} (GeV/#it{c});#it{#eta};#it{#varphi}", kTH3D, {axisPt, axisEta, axisPhi});
    histos.add("Tracks/RecoMCRad1mumto5mmCollMatch/histptFB4", "FB4;#it{p}_{T} (GeV/#it{c});#it{#eta};#it{#varphi}", kTH3D, {axisPt, axisEta, axisPhi});
    histos.add("Tracks/RecoMCRad1mumto5mmCollMatch/histptFB5", "FB5;#it{p}_{T} (GeV/#it{c});#it{#eta};#it{#varphi}", kTH3D, {axisPt, axisEta, axisPhi});
    histos.add("Tracks/RecoMCRad1mumto5mmCollMatch/histptITSonly", "ITSonly;#it{p}_{T} (GeV/#it{c});#it{#eta};#it{#varphi}", kTH3D, {axisPt, axisEta, axisPhi});
    histos.add("Tracks/RecoMCRad1mumto5mmCollMatch/histptTPConly", "TPConly;#it{p}_{T} (GeV/#it{c});#it{#eta};#it{#varphi}", kTH3D, {axisPt, axisEta, axisPhi});

    histos.add("Tracks/RecoMCfromHFdecayCollMatch/histptFB0", "FB0;#it{p}_{T} (GeV/#it{c});#it{#eta};#it{#varphi}", kTH3D, {axisPt, axisEta, axisPhi});
    histos.add("Tracks/RecoMCfromHFdecayCollMatch/histptFB1", "FB1;#it{p}_{T} (GeV/#it{c});#it{#eta};#it{#varphi}", kTH3D, {axisPt, axisEta, axisPhi});
    histos.add("Tracks/RecoMCfromHFdecayCollMatch/histptFB2", "FB2;#it{p}_{T} (GeV/#it{c});#it{#eta};#it{#varphi}", kTH3D, {axisPt, axisEta, axisPhi});
    histos.add("Tracks/RecoMCfromHFdecayCollMatch/histptFB3", "FB3;#it{p}_{T} (GeV/#it{c});#it{#eta};#it{#varphi}", kTH3D, {axisPt, axisEta, axisPhi});
    histos.add("Tracks/RecoMCfromHFdecayCollMatch/histptFB4", "FB4;#it{p}_{T} (GeV/#it{c});#it{#eta};#it{#varphi}", kTH3D, {axisPt, axisEta, axisPhi});
    histos.add("Tracks/RecoMCfromHFdecayCollMatch/histptFB5", "FB5;#it{p}_{T} (GeV/#it{c});#it{#eta};#it{#varphi}", kTH3D, {axisPt, axisEta, axisPhi});
    histos.add("Tracks/RecoMCfromHFdecayCollMatch/histptITSonly", "ITSonly;#it{p}_{T} (GeV/#it{c});#it{#eta};#it{#varphi}", kTH3D, {axisPt, axisEta, axisPhi});
    histos.add("Tracks/RecoMCfromHFdecayCollMatch/histptTPConly", "TPConly;#it{p}_{T} (GeV/#it{c});#it{#eta};#it{#varphi}", kTH3D, {axisPt, axisEta, axisPhi});


    //  histos.add("Pairs/PosITSposTPC",
    
    // plotting variables
    //    HistogramConfigSpec defaultVariableHist({HistType::kTHnF, {axisPt, axisEta, axisPhi,axisNclustTPC}});
    // histos.add("nclustTPC", "Sigmas", defaultParticleHist);
    histos.add("Tracks/Reco/histNclustTPC", "N clusters TPC;#it{p}_{T} (GeV/#it{c});#it{#eta};#it{#varphi};NclustTPCl;TPCITSmatching", HistType::kTHnF, {axisPt, axisEta, axisPhi, axisNclustTPC, binsTPCITSmatching});
    histos.add("Tracks/RecoMCVariablesPrimary/histNclustTPC", "N clusters TPC;#it{p}_{T} (GeV/#it{c});#it{#eta};#it{#varphi};NclustTPC;TPCITSmatching", HistType::kTHnF, {axisPt, axisEta, axisPhi, axisNclustTPC, binsTPCITSmatching});
  
    // trackselectionITSonly.ITSonlySetRequireHitsInITSLayers(7, {0, 1, 2, 3, 4, 5, 6});
  }

  void FillDataHistoStd(const Tracksextension::iterator &track){
     if (std::abs(track.eta()) < 0.9) {
        histos.fill(HIST("Tracks/Reco/histptAll"), track.pt());
      }
      histos.fill(HIST("Tracks/Reco/histpt3DAll"), track.pt(), track.eta(), track.phi());
      Int_t hasITS = 0;
      if (track.itsNCls() > 0)
        hasITS++;
      if (track.itsNClsInnerBarrel() > 0)
        hasITS++;
      histos.fill(HIST("Tracks/Reco/histNclustTPC"), track.pt(), track.eta(), track.phi(), track.tpcNClsFound(), hasITS);
      
      //Track prop without any TPC cuts
      if(track.passedTrackType() && track.passedITSNCls() && track.passedITSChi2NDF() && track.passedITSRefit() && track.passedITSHits() && track.passedTPCRefit()){
          histos.fill(HIST("Tracks/Reco/histptNewConfig"), track.pt(), track.eta(), track.phi());
          if(std::abs(track.eta()) < 0.9){
              //Fill different track properties
              histos.fill(HIST("Tracks/Reco/histNewConfigTpcNCls"), track.pt(), track.tpcNClsFound());
              histos.fill(HIST("Tracks/Reco/histNewConfigTpcNClsCrossRows"), track.pt(), track.tpcNClsCrossedRows());
              histos.fill(HIST("Tracks/Reco/histNewConfigTpcNClsCrossRowsOvrFindableCls"), track.pt(), track.tpcCrossedRowsOverFindableCls());
              histos.fill(HIST("Tracks/Reco/histNewConfigTpcChi2NCls"), track.pt(), track.tpcChi2NCl());
              histos.fill(HIST("Tracks/Reco/histNewConfigDcaXY"), track.pt(), track.dcaXY());
              histos.fill(HIST("Tracks/Reco/histNewConfigDcaZ"), track.pt(), track.dcaZ());
          }
      }
     
      if(track.isGlobalTrackWoDCA()){
        histos.fill(HIST("Tracks/Reco/histptGbNoDca"), track.pt(), track.eta(), track.phi());
        if(std::abs(track.eta()) < 0.9){
          //Fill different track properties 
          histos.fill(HIST("Tracks/Reco/histGbTrkTpcNCls"), track.pt(), track.tpcNClsFound()); 
          histos.fill(HIST("Tracks/Reco/histGbTrkTpcNClsCrossRows"), track.pt(), track.tpcNClsCrossedRows());
          histos.fill(HIST("Tracks/Reco/histGbTrkTpcNClsCrossRowsOvrFindableCls"), track.pt(), track.tpcCrossedRowsOverFindableCls()); 
          histos.fill(HIST("Tracks/Reco/histGbTrkTpcChi2NCls"), track.pt(), track.tpcChi2NCl()); 
          histos.fill(HIST("Tracks/Reco/histGbTrkItsNCls"), track.pt(), track.itsNCls()); 
          histos.fill(HIST("Tracks/Reco/histGbTrkItsChi2NCls"), track.pt(), track.itsChi2NCl()); 
          histos.fill(HIST("Tracks/Reco/histGbTrkDcaXY"), track.pt(), track.dcaXY()); 
          histos.fill(HIST("Tracks/Reco/histGbTrkDcaZ"), track.pt(), track.dcaZ()); 
        }
      }

      if (track.isGlobalTrack())
        histos.fill(HIST("Tracks/Reco/histpt3DFB0"), track.pt(), track.eta(), track.phi());
      if (track.trackCutFlagFb1())
        histos.fill(HIST("Tracks/Reco/histpt3DFB1"), track.pt(), track.eta(), track.phi());
      if (track.trackCutFlagFb2())
        histos.fill(HIST("Tracks/Reco/histpt3DFB2"), track.pt(), track.eta(), track.phi());
      if (track.trackCutFlagFb3())
        histos.fill(HIST("Tracks/Reco/histpt3DFB3"), track.pt(), track.eta(), track.phi());
      if (track.trackCutFlagFb4())
        histos.fill(HIST("Tracks/Reco/histpt3DFB4"), track.pt(), track.eta(), track.phi());
      if (track.trackCutFlagFb5())
        histos.fill(HIST("Tracks/Reco/histpt3DFB5"), track.pt(), track.eta(), track.phi());
      if (track.itsChi2NCl()>0. && track.tpcChi2NCl()<0.)
        histos.fill(HIST("Tracks/Reco/histpt3DITSonly"), track.pt(), track.eta(), track.phi());
      if (track.itsChi2NCl()<0. && track.tpcChi2NCl()>0.)
        histos.fill(HIST("Tracks/Reco/histpt3DTPConly"), track.pt(), track.eta(), track.phi());
  }

  void processData(soa::Join<o2::aod::Collisions, aod::EvSels>::iterator const& collision, Tracksextension const& tracks)
  { // test whether the name can be changed using the PROCESS_SWITCH method (see qaEventTrack class)

    if(!collision.sel8()) 
      return;

    histos.fill(HIST("EventProp/histDatacollZ"), collision.posZ());
    if (std::abs(collision.posZ()) > 10.)
      return;

    histos.fill(HIST("EventProp/histDataNEvents"), 0.5); //event counter

    for (auto& track : tracks) {
      FillDataHistoStd(track);
    }
  }
  PROCESS_SWITCH(CheckFilterBit, processData, "process data", true);

  void processDataCombineTracks(soa::Join<o2::aod::Collisions, aod::EvSels>::iterator const& collision,Tracksextension const& tracks)
  {
    if (!collision.sel8() || std::abs(collision.posZ()) > 10.){
      return;
    }

    auto positiveITSonlyTracksThisColl=positiveITSonlyTracks->sliceByCached(aod::track::collisionId,collision.globalIndex(), cache);
    auto negativeITSonlyTracksThisColl=negativeITSonlyTracks->sliceByCached(aod::track::collisionId,collision.globalIndex(), cache);
    auto positiveTPConlyTracksThisColl=positiveTPConlyTracks->sliceByCached(aod::track::collisionId,collision.globalIndex(), cache);
    auto negativeTPConlyTracksThisColl=negativeTPConlyTracks->sliceByCached(aod::track::collisionId,collision.globalIndex(), cache);

    processPair<false>(collision,positiveTPConlyTracksThisColl,positiveITSonlyTracksThisColl,negativeTPConlyTracksThisColl,negativeITSonlyTracksThisColl);
    // processPair<false>(collision,tracks);
  }
  PROCESS_SWITCH(CheckFilterBit, processDataCombineTracks, "process data combined tracks", false);

  void processMCCombineTracks(soa::Join<aod::Collisions, o2::aod::McCollisionLabels, aod::EvSels>::iterator const& collision,TracksextensionMC const& tracks,aod::McParticles const& mcParticles)
  {
    if (!collision.sel8() || std::abs(collision.posZ()) > 10.){
      return;
    }
      
    auto positiveITSonlyTracksThisColl=positiveITSonlyTracksMC->sliceByCached(aod::track::collisionId,collision.globalIndex(), cache);
    auto negativeITSonlyTracksThisColl=negativeITSonlyTracksMC->sliceByCached(aod::track::collisionId,collision.globalIndex(), cache);
    auto positiveTPConlyTracksThisColl=positiveTPConlyTracksMC->sliceByCached(aod::track::collisionId,collision.globalIndex(), cache);
    auto negativeTPConlyTracksThisColl=negativeTPConlyTracksMC->sliceByCached(aod::track::collisionId,collision.globalIndex(), cache);

   
    processPair<true>(collision,positiveTPConlyTracksThisColl,positiveITSonlyTracksThisColl,negativeTPConlyTracksThisColl,negativeITSonlyTracksThisColl);
  }
  PROCESS_SWITCH(CheckFilterBit, processMCCombineTracks, "process data combined tracks", true);

  template <bool IS_MC, typename Tcollisions,typename TpTPC, typename TpITS, typename TnTPC, typename TnITS>
  void processPair(Tcollisions const& collision,const TpTPC& positiveTPConlyTracksThisColl, const TpITS& positiveITSonlyTracksThisColl, const TnTPC& negativeTPConlyTracksThisColl,const TnITS& negativeITSonlyTracksThisColl){
   float issamemcpt=-9999.;
    // run first over global tracks
    /*  for (auto& track : tracks) {
      if (track.isGlobalTrack()) {
    track.pt();
      }
      else{
    track.pt();
      }
    }
    */
    
    for (auto& [track0, track1] : combinations(soa::CombinationsFullIndexPolicy(positiveITSonlyTracksThisColl, positiveTPConlyTracksThisColl))){
      issamemcpt=-9999.;
          if constexpr (IS_MC) {
        
        if(track0.mcParticleId()==track1.mcParticleId()){
          if (track0.has_mcParticle()) {
        // the track is not fake
        auto mcparticle = track0.mcParticle();
        auto mcCollID_recoColl = collision.mcCollisionId();
        auto mcCollID_particle = mcparticle.mcCollisionId();
        bool indexMatchOK = (mcCollID_recoColl == mcCollID_particle);

        int partpdg = std::abs(mcparticle.pdgCode());
        if (indexMatchOK && mcparticle.isPhysicalPrimary() && (partpdg == 211 || partpdg == 321 || partpdg == 2212 || partpdg == 11 || partpdg == 13)) {
          issamemcpt=mcparticle.pt();
        }
        else issamemcpt=-mcparticle.pt();
          }
          else {Printf("track MC matched but not has_mcParticle");}
          
          
        }
      }

      trackPairForEffTablePP(track1.pt(),track1.eta(),track1.phi(),track0.pt(),track0.eta(),track0.phi(),track0.itsNCls(),track1.tpcNClsFound(),issamemcpt);
    }
    for (auto& [track0, track1] : combinations(soa::CombinationsFullIndexPolicy(negativeITSonlyTracksThisColl, negativeTPConlyTracksThisColl))){

      issamemcpt=-9999.;
          if constexpr (IS_MC) {
        if(track0.mcParticleId()==track1.mcParticleId()){
          if (track0.has_mcParticle()) {
        /// the track is not fake
        auto mcparticle = track0.mcParticle();
        auto mcCollID_recoColl = collision.mcCollisionId();
        auto mcCollID_particle = mcparticle.mcCollisionId();
        bool indexMatchOK = (mcCollID_recoColl == mcCollID_particle);

        int partpdg = std::abs(mcparticle.pdgCode());
        if (indexMatchOK && mcparticle.isPhysicalPrimary() && (partpdg == 211 || partpdg == 321 || partpdg == 2212 || partpdg == 11 || partpdg == 13)) {
          issamemcpt=mcparticle.pt();
        }
        else issamemcpt=-mcparticle.pt();
          }
        }
          
          
      }
      
      trackPairForEffTableNN(track1.pt(),track1.eta(),track1.phi(),track0.pt(),track0.eta(),track0.phi(),track0.itsNCls(),track1.tpcNClsFound(),issamemcpt);
    }
    for (auto& [track0, track1] : combinations(soa::CombinationsFullIndexPolicy(negativeITSonlyTracksThisColl,positiveTPConlyTracksThisColl))){
      issamemcpt=-9999.;
        if constexpr (IS_MC) {
          if(track0.mcParticleId()==track1.mcParticleId()){
            if (track0.has_mcParticle()) {
            /// the track is not fake
              auto mcparticle = track0.mcParticle();
              auto mcCollID_recoColl = collision.mcCollisionId();
              auto mcCollID_particle = mcparticle.mcCollisionId();
              bool indexMatchOK = (mcCollID_recoColl == mcCollID_particle);

              int partpdg = std::abs(mcparticle.pdgCode());
              if (indexMatchOK && mcparticle.isPhysicalPrimary() && (partpdg == 211 || partpdg == 321 || partpdg == 2212 || partpdg == 11 || partpdg == 13)) {
              issamemcpt=mcparticle.pt();
              }
              else issamemcpt=-mcparticle.pt();
            }
          } 
        }
      trackPairForEffTableNP(track1.pt(),track1.eta(),track1.phi(),track0.pt(),track0.eta(),track0.phi(),track0.itsNCls(),track1.tpcNClsFound(),issamemcpt);
    }
    for (auto& [track0, track1] : combinations(soa::CombinationsFullIndexPolicy(positiveITSonlyTracksThisColl,negativeTPConlyTracksThisColl))){
      //Printf("Collision %lld Positive ITS (clusters %d, %d) Negative TPC tracks, indices %lld, %lld, %lld, %lld",collision.globalIndex(),track0.itsNClsInnerBarrel(),track0.itsNCls(),track0.globalIndex(),track0.index(),track1.globalIndex(),track1.index());
      issamemcpt=-9999.;
          if constexpr (IS_MC) {
        if(track0.mcParticleId()==track1.mcParticleId()){
          if (track0.has_mcParticle()) {
        /// the track is not fake
        auto mcparticle = track0.mcParticle();
                auto mcCollID_recoColl = collision.mcCollisionId();
        auto mcCollID_particle = mcparticle.mcCollisionId();
        bool indexMatchOK = (mcCollID_recoColl == mcCollID_particle);

        int partpdg = std::abs(mcparticle.pdgCode());
        if (indexMatchOK && mcparticle.isPhysicalPrimary() && (partpdg == 211 || partpdg == 321 || partpdg == 2212 || partpdg == 11 || partpdg == 13)) {
          issamemcpt=mcparticle.pt();
        }
        else issamemcpt=-mcparticle.pt();
          }
        }
      }
      trackPairForEffTablePN(track1.pt(),track1.eta(),track1.phi(),track0.pt(),track0.eta(),track0.phi(),track0.itsNCls(),track1.tpcNClsFound(),issamemcpt);
    }
  }

  template <typename T>
  int isFromStrangeDecay(const T& particlesMC, const typename T::iterator& particle)
  {

    std::vector<std::vector<int64_t>> arrayIds{};
    std::vector<int64_t> initVec{particle.globalIndex()};
    arrayIds.push_back(initVec);

    int stage = 0;
    int strangeness = 0;
    while (stage < 4 && arrayIds[stage].size() > 0 && strangeness == 0) {
      std::vector<int64_t> arrayIdsStage{};
      for (auto& iPart : arrayIds[stage]) {
        auto particleMother = particlesMC.rawIteratorAt(iPart - particlesMC.offset());
        if (particleMother.has_mothers()) {
          for (auto iMother = particleMother.mothersIds().front(); iMother <= particleMother.mothersIds().back(); ++iMother) {
            if (std::find(arrayIdsStage.begin(), arrayIdsStage.end(), iMother) != arrayIdsStage.end()) {
              continue;
            }
            auto mother = particlesMC.rawIteratorAt(iMother - particlesMC.offset());
            auto motherPDG = std::abs(mother.pdgCode());
            if (motherPDG / 1000 == 3) { // strange baryon
              strangeness = 1;
              break;
            } else if (motherPDG / 1000 == 0 && motherPDG / 100 == 3) {
              strangeness = 1;
              break;
            }
            arrayIdsStage.push_back(iMother);
          }
        }
        arrayIds.push_back(arrayIdsStage);
      }
      stage++;
    }
    return strangeness;
  }
  
  void FillRecoMCHisto(const TracksextensionMC::iterator &track){
    if(std::abs(track.eta()) < 0.9){
      //Track prop without TPC cuts
      if(track.passedTrackType() && track.passedITSNCls() && track.passedITSChi2NDF() && track.passedITSRefit() && track.passedITSHits() && track.passedTPCRefit()){
        histos.fill(HIST("Tracks/RecoMC/histptNewConfig"), track.pt(), track.eta(), track.phi());
        //Fill different track properties
        histos.fill(HIST("Tracks/RecoMC/histNewConfigTpcNCls"), track.pt(), track.tpcNClsFound());
        histos.fill(HIST("Tracks/RecoMC/histNewConfigTpcNClsCrossRows"), track.pt(), track.tpcNClsCrossedRows());
        histos.fill(HIST("Tracks/RecoMC/histNewConfigTpcNClsCrossRowsOvrFindableCls"), track.pt(), track.tpcCrossedRowsOverFindableCls());
        histos.fill(HIST("Tracks/RecoMC/histNewConfigTpcChi2NCls"), track.pt(), track.tpcChi2NCl());
        histos.fill(HIST("Tracks/RecoMC/histNewConfigDcaXY"), track.pt(), track.dcaXY());
        histos.fill(HIST("Tracks/RecoMC/histNewConfigDcaZ"), track.pt(), track.dcaZ());
      }

      //Phys Primary tracks
      bool has_MCparticle = track.has_mcParticle();
      if (has_MCparticle) { 
        auto mcparticle = track.mcParticle();
        int partpdg = std::abs(mcparticle.pdgCode());
        if (partpdg == 211 || partpdg == 321 || partpdg == 2212 || partpdg == 11 || partpdg == 13) {
          if (mcparticle.isPhysicalPrimary()) {
            //Track prop without TPC cuts 
            if(track.passedTrackType() && track.passedITSNCls() && track.passedITSChi2NDF() && track.passedITSRefit() && track.passedITSHits() && track.passedTPCRefit()){
              histos.fill(HIST("Tracks/RecoMCPhysPrim/histptNewConfig"), track.pt(), track.eta(), track.phi());
              histos.fill(HIST("Tracks/RecoMCPhysPrim/histNewConfigTpcNCls"), track.pt(), track.tpcNClsFound());
              histos.fill(HIST("Tracks/RecoMCPhysPrim/histNewConfigTpcNClsCrossRows"), track.pt(), track.tpcNClsCrossedRows());
              histos.fill(HIST("Tracks/RecoMCPhysPrim/histNewConfigTpcNClsCrossRowsOvrFindableCls"), track.pt(), track.tpcCrossedRowsOverFindableCls());
              histos.fill(HIST("Tracks/RecoMCPhysPrim/histNewConfigTpcChi2NCls"), track.pt(), track.tpcChi2NCl());
              histos.fill(HIST("Tracks/RecoMCPhysPrim/histNewConfigDcaXY"), track.pt(), track.dcaXY());
              histos.fill(HIST("Tracks/RecoMCPhysPrim/histNewConfigDcaZ"), track.pt(), track.dcaZ());
            }
          }
        }
      }
    }//eta cut
  }
  void processRecoMC(soa::Join<aod::Collisions, aod::McCollisionLabels, aod::EvSels>::iterator const& collision, TracksextensionMC const& tracks, aod::McParticles const& mcParticles, aod::McCollisions const& mcCollisions)
  {
  //  if(!collision.sel8()) 
  //    return;
      
    histos.fill(HIST("EventProp/histRecoMCcollZ"), collision.posZ());
    if (std::abs(collision.posZ()) > 10.)
      return;
    ncollisionCounter++;
    histos.fill(HIST("EventProp/histRecoMcNEvents"), 0.5); //event counter
    
    for (auto& track : tracks) {
      FillRecoMCHisto(track); //Fill all reco track properties
      if (track.collisionId() < 0)
        histos.fill(HIST("EventProp/histPtTrackNegCollID"), track.pt());
      bool has_MCparticle = track.has_mcParticle();
      if (has_MCparticle) {
        /// the track is not fake
        auto mcparticle = track.mcParticle();
        // auto collReco = track.collision_as<CollisionTableMC>();
        auto collMC = mcparticle.mcCollision();
        auto mcCollID_recoColl = collision.mcCollisionId();
        auto mcCollID_particle = mcparticle.mcCollisionId();
        bool indexMatchOK = (mcCollID_recoColl == mcCollID_particle);
        // double pvZdiff = collision.posZ() - collMC.posZ();
        if (indexMatchOK) {
          double prodRadius2 = (mcparticle.vx() - collMC.posX()) * (mcparticle.vx() - collMC.posX()) + (mcparticle.vy() - collMC.posY()) * (mcparticle.vy() - collMC.posY());
          int partpdg = std::abs(mcparticle.pdgCode());
          if (partpdg == 211 || partpdg == 321 || partpdg == 2212 || partpdg == 11 || partpdg == 13) {
            int isFromStrange = isFromStrangeDecay(mcParticles, mcparticle);
            if (mcparticle.isPhysicalPrimary()) {
              int isHF = RecoDecay::getCharmHadronOrigin(mcParticles, mcparticle, false);
              if (std::abs(track.eta()) < 0.9) {
                histos.fill(HIST("Tracks/RecoMCPhysPrimCollMatch/histpt"), mcparticle.pt()); // note: one needs to avoid double counting of tracks reco both in TPC and ITS but not matched
              }
              Int_t hasITS = 0;
              if (track.itsNCls() > 0.)
                hasITS++;
              if (track.itsNClsInnerBarrel() > 0.)
                hasITS++;
              //histos.fill(HIST("Tracks/RecoMCVariablesPrimary/histNclustTPC"), track.pt(), track.eta(), track.phi(), track.tpcNClsFound(), hasITS);
                
              //Track prop without any TPC cuts
              if(track.passedTrackType() && track.passedITSNCls() && track.passedITSChi2NDF() && track.passedITSRefit() && track.passedITSHits() && track.passedTPCRefit()){
                  histos.fill(HIST("Tracks/RecoMCPhysPrimCollMatch/histptNewConfig"), track.pt(), track.eta(), track.phi());
                  if(std::abs(track.eta()) < 0.9){
                    //Fill different track properties
                    histos.fill(HIST("Tracks/RecoMCPhysPrimCollMatch/histNewConfigTpcNCls"), track.pt(), track.tpcNClsFound());
                    histos.fill(HIST("Tracks/RecoMCPhysPrimCollMatch/histNewConfigTpcNClsCrossRows"), track.pt(), track.tpcNClsCrossedRows());
                    histos.fill(HIST("Tracks/RecoMCPhysPrimCollMatch/histNewConfigTpcNClsCrossRowsOvrFindableCls"), track.pt(), track.tpcCrossedRowsOverFindableCls());
                    histos.fill(HIST("Tracks/RecoMCPhysPrimCollMatch/histNewConfigTpcChi2NCls"), track.pt(), track.tpcChi2NCl());
                    histos.fill(HIST("Tracks/RecoMCPhysPrimCollMatch/histNewConfigDcaXY"), track.pt(), track.dcaXY());
                    histos.fill(HIST("Tracks/RecoMCPhysPrimCollMatch/histNewConfigDcaZ"), track.pt(), track.dcaZ());
                  }
              }

              if(track.isGlobalTrackWoDCA()) {
                histos.fill(HIST("Tracks/RecoMCPhysPrimCollMatch/histptGbNoDca"), track.pt(), track.eta(), track.phi());
                if(std::abs(track.eta()) < 0.9){
                  //Fill different track properties 
                  histos.fill(HIST("Tracks/RecoMCPhysPrimCollMatch/histGbTrkTpcNCls"), track.pt(), track.tpcNClsFound()); 
                  histos.fill(HIST("Tracks/RecoMCPhysPrimCollMatch/histGbTrkTpcNClsCrossRows"), track.pt(), track.tpcNClsCrossedRows());
                  histos.fill(HIST("Tracks/RecoMCPhysPrimCollMatch/histGbTrkTpcNClsCrossRowsOvrFindableCls"), track.pt(), track.tpcCrossedRowsOverFindableCls()); 
                  histos.fill(HIST("Tracks/RecoMCPhysPrimCollMatch/histGbTrkTpcChi2NCls"), track.pt(), track.tpcChi2NCl()); 
                  histos.fill(HIST("Tracks/RecoMCPhysPrimCollMatch/histGbTrkItsNCls"), track.pt(), track.itsNCls()); 
                  histos.fill(HIST("Tracks/RecoMCPhysPrimCollMatch/histGbTrkItsChi2NCls"), track.pt(), track.itsChi2NCl()); 
                  histos.fill(HIST("Tracks/RecoMCPhysPrimCollMatch/histGbTrkDcaXY"), track.pt(), track.dcaXY()); 
                  histos.fill(HIST("Tracks/RecoMCPhysPrimCollMatch/histGbTrkDcaZ"), track.pt(), track.dcaZ()); 
                }
              }

              if (track.isGlobalTrack()) {
                histos.fill(HIST("Tracks/RecoMCPhysPrimCollMatch/histptFB0"), track.pt(), track.eta(), track.phi());
                histos.fill(HIST("Tracks/RecoMCPhysPrimCollMatch/histptMCFB0"), mcparticle.pt(), track.eta(), track.phi());
              }
              if (track.itsChi2NCl()>0. && track.tpcChi2NCl()<0.){
                histos.fill(HIST("Tracks/RecoMCPhysPrimCollMatch/histptITSonly"), track.pt(), track.eta(), track.phi());
                histos.fill(HIST("Tracks/RecoMCPhysPrimCollMatch/histptMCITSonly"), mcparticle.pt(), track.eta(), track.phi());
              }
              else if (track.itsChi2NCl()<0. && track.tpcChi2NCl()>0.){
                histos.fill(HIST("Tracks/RecoMCPhysPrimCollMatch/histptTPConly"), track.pt(), track.eta(), track.phi());
                histos.fill(HIST("Tracks/RecoMCPhysPrimCollMatch/histptMCTPConly"), mcparticle.pt(), track.eta(), track.phi());

                histos.fill(HIST("Tracks/RecoMCPhysPrimCollMatch/histptMCTPConlyWithClusters"),mcparticle.pt(), track.eta(), track.phi(),track.tpcNClsFound());
                histos.fill(HIST("Tracks/RecoMCPhysPrimCollMatch/histptTPConlyWithClusters"),track.pt(), track.eta(), track.phi(),track.tpcNClsFound());
              }
              if (track.trackCutFlagFb1()) {
                histos.fill(HIST("Tracks/RecoMCPhysPrimCollMatch/histptFB1"), track.pt(), track.eta(), track.phi());
                histos.fill(HIST("Tracks/RecoMCPhysPrimCollMatch/histptMCFB1"), mcparticle.pt(), track.eta(), track.phi());
              }
              if (track.trackCutFlagFb2()) {
                histos.fill(HIST("Tracks/RecoMCPhysPrimCollMatch/histptFB2"), track.pt(), track.eta(), track.phi());
                histos.fill(HIST("Tracks/RecoMCPhysPrimCollMatch/histptMCFB2"), mcparticle.pt(), track.eta(), track.phi());
              }
              if (track.trackCutFlagFb3()) {
                histos.fill(HIST("Tracks/RecoMCPhysPrimCollMatch/histptFB3"), track.pt(), track.eta(), track.phi());
                histos.fill(HIST("Tracks/RecoMCPhysPrimCollMatch/histptMCFB3"), mcparticle.pt(), track.eta(), track.phi());
              }
              if (track.trackCutFlagFb4()) {
                histos.fill(HIST("Tracks/RecoMCPhysPrimCollMatch/histptFB4"), track.pt(), track.eta(), track.phi());
                histos.fill(HIST("Tracks/RecoMCPhysPrimCollMatch/histptMCFB4"), mcparticle.pt(), track.eta(), track.phi());
              }
              if (track.trackCutFlagFb5()) {
                histos.fill(HIST("Tracks/RecoMCPhysPrimCollMatch/histptFB5"), track.pt(), track.eta(), track.phi());
                histos.fill(HIST("Tracks/RecoMCPhysPrimCollMatch/histptMCFB5"), mcparticle.pt(), track.eta(), track.phi());
              }
              if (isHF == RecoDecay::OriginType::Prompt || isHF == RecoDecay::OriginType::NonPrompt) {
                if (track.isGlobalTrack()) {
                  histos.fill(HIST("Tracks/RecoMCfromHFdecayCollMatch/histptFB0"), track.pt(), track.eta(), track.phi());
                }
                if(track.itsChi2NCl()>0. && track.tpcChi2NCl()<0.){
                histos.fill(HIST("Tracks/RecoMCfromHFdecayCollMatch/histptITSonly"), track.pt(), track.eta(), track.phi());
                }
                else if(track.itsChi2NCl()<0. && track.tpcChi2NCl()>0.){
                  histos.fill(HIST("Tracks/RecoMCfromHFdecayCollMatch/histptTPConly"), track.pt(), track.eta(), track.phi());
                }
                if (track.trackCutFlagFb1()) {
                  histos.fill(HIST("Tracks/RecoMCfromHFdecayCollMatch/histptFB1"), track.pt(), track.eta(), track.phi());
                }
                if (track.trackCutFlagFb2()) {
                  histos.fill(HIST("Tracks/RecoMCfromHFdecayCollMatch/histptFB2"), track.pt(), track.eta(), track.phi());
                }
                if (track.trackCutFlagFb3()) {
                  histos.fill(HIST("Tracks/RecoMCfromHFdecayCollMatch/histptFB3"), track.pt(), track.eta(), track.phi());
                }
                if (track.trackCutFlagFb4()) {
                  histos.fill(HIST("Tracks/RecoMCfromHFdecayCollMatch/histptFB4"), track.pt(), track.eta(), track.phi());
                }
                if (track.trackCutFlagFb5()) {
                  histos.fill(HIST("Tracks/RecoMCfromHFdecayCollMatch/histptFB5"), track.pt(), track.eta(), track.phi());
                }
              }
            } 
            else if (prodRadius2 > 1. && prodRadius2 < 225. && isFromStrange) {
              if (track.isGlobalTrack())
                histos.fill(HIST("Tracks/RecoMCRad1to15cmCollMatch/histptFB0"), track.pt(), track.eta(), track.phi());
              if(track.itsChi2NCl()>0. && track.tpcChi2NCl()<0.){
                histos.fill(HIST("Tracks/RecoMCRad1to15cmCollMatch/histptITSonly"), track.pt(), track.eta(), track.phi());
              }
              else if (track.itsChi2NCl()<0. && track.tpcChi2NCl()>0.){
                histos.fill(HIST("Tracks/RecoMCRad1to15cmCollMatch/histptTPConly"), track.pt(), track.eta(), track.phi());
              }
              if (track.trackCutFlagFb1())
                histos.fill(HIST("Tracks/RecoMCRad1to15cmCollMatch/histptFB1"), track.pt(), track.eta(), track.phi());
              if (track.trackCutFlagFb2())
                histos.fill(HIST("Tracks/RecoMCRad1to15cmCollMatch/histptFB2"), track.pt(), track.eta(), track.phi());
              if (track.trackCutFlagFb3())
                histos.fill(HIST("Tracks/RecoMCRad1to15cmCollMatch/histptFB3"), track.pt(), track.eta(), track.phi());
              if (track.trackCutFlagFb4())
                histos.fill(HIST("Tracks/RecoMCRad1to15cmCollMatch/histptFB4"), track.pt(), track.eta(), track.phi());
              if (track.trackCutFlagFb5())
                histos.fill(HIST("Tracks/RecoMCRad1to15cmCollMatch/histptFB5"), track.pt(), track.eta(), track.phi());
            }
            if (prodRadius2 > 1.e-8 && prodRadius2 < 0.25) {
              if (track.isGlobalTrack())
                histos.fill(HIST("Tracks/RecoMCRad1mumto5mmCollMatch/histptFB0"), track.pt(), track.eta(), track.phi());
              if(track.itsChi2NCl()>0. && track.tpcChi2NCl()<0.){
                histos.fill(HIST("Tracks/RecoMCRad1mumto5mmCollMatch/histptITSonly"), track.pt(), track.eta(), track.phi());
              }
              else if (track.itsChi2NCl()<0. && track.tpcChi2NCl()>0.){
                histos.fill(HIST("Tracks/RecoMCRad1mumto5mmCollMatch/histptTPConly"), track.pt(), track.eta(), track.phi());
              }
              if (track.trackCutFlagFb1())
                histos.fill(HIST("Tracks/RecoMCRad1mumto5mmCollMatch/histptFB1"), track.pt(), track.eta(), track.phi());
              if (track.trackCutFlagFb2())
                histos.fill(HIST("Tracks/RecoMCRad1mumto5mmCollMatch/histptFB2"), track.pt(), track.eta(), track.phi());
              if (track.trackCutFlagFb3())
                histos.fill(HIST("Tracks/RecoMCRad1mumto5mmCollMatch/histptFB3"), track.pt(), track.eta(), track.phi());
              if (track.trackCutFlagFb4())
                histos.fill(HIST("Tracks/RecoMCRad1mumto5mmCollMatch/histptFB4"), track.pt(), track.eta(), track.phi());
              if (track.trackCutFlagFb5())
                histos.fill(HIST("Tracks/RecoMCRad1mumto5mmCollMatch/histptFB5"), track.pt(), track.eta(), track.phi());
            }
          }
        }
      }
    }
  }
  PROCESS_SWITCH(CheckFilterBit, processRecoMC, "process mc", true);

  void processMC(aod::McCollision const& mcCollision, aod::McParticles const& mcParticles)
  {

    histos.fill(HIST("EventProp/histMCcollZ"), mcCollision.posZ());
    if (std::abs(mcCollision.posZ()) > 10.)
      return;
    ncollisionCounter++;

    histos.fill(HIST("EventProp/histGenMcNEvents"), 0.5); //event counter
    
    for (auto& mcpart : mcParticles) {
      // if(!mcpart.producedByGenerator())continue;
      double prodRadius2 = (mcpart.vx() - mcCollision.posX()) * (mcpart.vx() - mcCollision.posX()) + (mcpart.vy() - mcCollision.posY()) * (mcpart.vy() - mcCollision.posY());
      if (std::abs(mcpart.pdgCode()) == 211 || std::abs(mcpart.pdgCode()) == 321 || std::abs(mcpart.pdgCode()) == 2212 || std::abs(mcpart.pdgCode()) == 11 || std::abs(mcpart.pdgCode()) == 13) {
        int isHF = RecoDecay::getCharmHadronOrigin(mcParticles, mcpart, false);
        int isFromStrange = isFromStrangeDecay(mcParticles, mcpart);
          
        if (mcpart.isPhysicalPrimary()) {
          if (std::abs(mcpart.eta()) < 0.9) {
            histos.fill(HIST("Tracks/MCgen/histMCgenpt"), mcpart.pt());
          }
          histos.fill(HIST("Tracks/MCgen/histMCgen3dPhysPrimary"), mcpart.pt(), mcpart.eta(), mcpart.phi());
          if (isHF == RecoDecay::OriginType::Prompt || isHF == RecoDecay::OriginType::NonPrompt)
            histos.fill(HIST("Tracks/MCgen/histMCgen3dChargedfromHFdecay"), mcpart.pt(), mcpart.eta(), mcpart.phi());
        } else if (prodRadius2 > 1. && prodRadius2 < 225. && isFromStrange) {
          histos.fill(HIST("Tracks/MCgen/histMCgen3dChargedProdRad1to15cm"), mcpart.pt(), mcpart.eta(), mcpart.phi());
        }
        if (prodRadius2 > 1.e-8 && prodRadius2 < 0.25) {
          histos.fill(HIST("Tracks/MCgen/histMCgen3dChargedProdRad1mumto5mm"), mcpart.pt(), mcpart.eta(), mcpart.phi());
        }
      }
    }
  }
  PROCESS_SWITCH(CheckFilterBit, processMC, "process mc", true);
};

WorkflowSpec defineDataProcessing(ConfigContext const& cfgc)
{
  return WorkflowSpec{
    adaptAnalysisTask<CheckFilterBit>(cfgc)

  };
}

// void customize(std::vector<o2::framework::ConfigParamSpec>& workflowOptions)
//{
//   ConfigParamSpec optionDoMC{"doMC", VariantType::Bool, false, {"Use MC info"}};
//   workflowOptions.push_back(optionDoMC);
// }
