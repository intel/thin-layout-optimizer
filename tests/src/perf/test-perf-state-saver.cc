#include "src/util/algo.h"

#include "gtest/gtest.h"

#include "src/util/vec.h"

#include "src/perf/perf-saver.h"
#include "src/perf/perf-stats.h"

#include <random>
#include <string>

#include "perf-file-and-save-state-helper.h"

#define INPUT_PATH     TLO_PROJECT_DIR "/tests/src/perf/test-inputs/partial-saves/"
#define ORDER_PATH_FMT INPUT_PATH "expec-order-c3"
struct partial_state_info_t {
    bool unknown_;
    // NOLINTBEGIN(cppcoreguidelines-avoid-const-or-ref-data-members)
    const char *                       expec_order_path_;
    const uint64_t                     expec_total_size_;
    const tlo::vec_t<std::string_view> saved_states_;
    // NOLINTEND(cppcoreguidelines-avoid-const-or-ref-data-members)
};

// NOLINTBEGIN(cppcoreguidelines-avoid-c-arrays,hicpp-avoid-c-arrays,modernize-avoid-c-arrays,bugprone-suspicious-missing-comma)
TLO_DISABLE_WEXIT_TIME_DESTRUCTORS
static const partial_state_info_t k_combinable_save_states[] = {
    { false,
      ORDER_PATH_FMT "-findable-2500",
      6394630,
      { INPUT_PATH "findable-partials-2500-xam.saved",
        INPUT_PATH "findable-partials-2500-xbj.saved",
        INPUT_PATH "findable-partials-2500-xak.saved",
        INPUT_PATH "findable-partials-2500-xad.saved",
        INPUT_PATH "findable-partials-2500-xav.saved",
        INPUT_PATH "findable-partials-2500-xaj.saved",
        INPUT_PATH "findable-partials-2500-xat.saved",
        INPUT_PATH "findable-partials-2500-xan.saved",
        INPUT_PATH "findable-partials-2500-xbf.saved",
        INPUT_PATH "findable-partials-2500-xau.saved",
        INPUT_PATH "findable-partials-2500-xab.saved",
        INPUT_PATH "findable-partials-2500-xao.saved",
        INPUT_PATH "findable-partials-2500-xbb.saved",
        INPUT_PATH "findable-partials-2500-xar.saved",
        INPUT_PATH "findable-partials-2500-xaf.saved",
        INPUT_PATH "findable-partials-2500-xba.saved",
        INPUT_PATH "findable-partials-2500-xaw.saved",
        INPUT_PATH "findable-partials-2500-xag.saved",
        INPUT_PATH "findable-partials-2500-xbi.saved",
        INPUT_PATH "findable-partials-2500-xbd.saved",
        INPUT_PATH "findable-partials-2500-xaz.saved",
        INPUT_PATH "findable-partials-2500-xax.saved",
        INPUT_PATH "findable-partials-2500-xah.saved",
        INPUT_PATH "findable-partials-2500-xaa.saved",
        INPUT_PATH "findable-partials-2500-xaq.saved",
        INPUT_PATH "findable-partials-2500-xbh.saved",
        INPUT_PATH "findable-partials-2500-xap.saved",
        INPUT_PATH "findable-partials-2500-xbc.saved",
        INPUT_PATH "findable-partials-2500-xac.saved",
        INPUT_PATH "findable-partials-2500-xai.saved",
        INPUT_PATH "findable-partials-2500-xbe.saved",
        INPUT_PATH "findable-partials-2500-xay.saved",
        INPUT_PATH "findable-partials-2500-xbg.saved",
        INPUT_PATH "findable-partials-2500-xas.saved",
        INPUT_PATH "findable-partials-2500-xae.saved",
        INPUT_PATH "findable-partials-2500-xal.saved" } },
    { false,
      ORDER_PATH_FMT "-findable-100",
      6394630,
      { INPUT_PATH "findable-partials-100-xql.saved",
        INPUT_PATH "findable-partials-100-xzaga.saved",
        INPUT_PATH "findable-partials-100-xmq.saved",
        INPUT_PATH "findable-partials-100-xpd.saved",
        INPUT_PATH "findable-partials-100-xzafd.saved",
        INPUT_PATH "findable-partials-100-xva.saved",
        INPUT_PATH "findable-partials-100-xzaid.saved",
        INPUT_PATH "findable-partials-100-xzaak.saved",
        INPUT_PATH "findable-partials-100-xzadz.saved",
        INPUT_PATH "findable-partials-100-xml.saved",
        INPUT_PATH "findable-partials-100-xct.saved",
        INPUT_PATH "findable-partials-100-xuq.saved",
        INPUT_PATH "findable-partials-100-xyr.saved",
        INPUT_PATH "findable-partials-100-xzadw.saved",
        INPUT_PATH "findable-partials-100-xor.saved",
        INPUT_PATH "findable-partials-100-xkn.saved",
        INPUT_PATH "findable-partials-100-xam.saved",
        INPUT_PATH "findable-partials-100-xzaca.saved",
        INPUT_PATH "findable-partials-100-xtq.saved",
        INPUT_PATH "findable-partials-100-xnn.saved",
        INPUT_PATH "findable-partials-100-xmd.saved",
        INPUT_PATH "findable-partials-100-xlz.saved",
        INPUT_PATH "findable-partials-100-xzaic.saved",
        INPUT_PATH "findable-partials-100-xzagj.saved",
        INPUT_PATH "findable-partials-100-xzafu.saved",
        INPUT_PATH "findable-partials-100-xnw.saved",
        INPUT_PATH "findable-partials-100-xks.saved",
        INPUT_PATH "findable-partials-100-xoh.saved",
        INPUT_PATH "findable-partials-100-xzagy.saved",
        INPUT_PATH "findable-partials-100-xxd.saved",
        INPUT_PATH "findable-partials-100-xve.saved",
        INPUT_PATH "findable-partials-100-xng.saved",
        INPUT_PATH "findable-partials-100-xep.saved",
        INPUT_PATH "findable-partials-100-xzaia.saved",
        INPUT_PATH "findable-partials-100-xcm.saved",
        INPUT_PATH "findable-partials-100-xwn.saved",
        INPUT_PATH "findable-partials-100-xlb.saved",
        INPUT_PATH "findable-partials-100-xce.saved",
        INPUT_PATH "findable-partials-100-xzaaz.saved",
        INPUT_PATH "findable-partials-100-xgx.saved",
        INPUT_PATH "findable-partials-100-xzaha.saved",
        INPUT_PATH "findable-partials-100-xzaib.saved",
        INPUT_PATH "findable-partials-100-xjo.saved",
        INPUT_PATH "findable-partials-100-xle.saved",
        INPUT_PATH "findable-partials-100-xjq.saved",
        INPUT_PATH "findable-partials-100-xta.saved",
        INPUT_PATH "findable-partials-100-xzafm.saved",
        INPUT_PATH "findable-partials-100-xws.saved",
        INPUT_PATH "findable-partials-100-xzaip.saved",
        INPUT_PATH "findable-partials-100-xis.saved",
        INPUT_PATH "findable-partials-100-xnv.saved",
        INPUT_PATH "findable-partials-100-xie.saved",
        INPUT_PATH "findable-partials-100-xbj.saved",
        INPUT_PATH "findable-partials-100-xzahs.saved",
        INPUT_PATH "findable-partials-100-xga.saved",
        INPUT_PATH "findable-partials-100-xgi.saved",
        INPUT_PATH "findable-partials-100-xzadh.saved",
        INPUT_PATH "findable-partials-100-xsz.saved",
        INPUT_PATH "findable-partials-100-xds.saved",
        INPUT_PATH "findable-partials-100-xrf.saved",
        INPUT_PATH "findable-partials-100-xqv.saved",
        INPUT_PATH "findable-partials-100-xrs.saved",
        INPUT_PATH "findable-partials-100-xzagv.saved",
        INPUT_PATH "findable-partials-100-xzain.saved",
        INPUT_PATH "findable-partials-100-xtn.saved",
        INPUT_PATH "findable-partials-100-xcn.saved",
        INPUT_PATH "findable-partials-100-xtt.saved",
        INPUT_PATH "findable-partials-100-xgl.saved",
        INPUT_PATH "findable-partials-100-xak.saved",
        INPUT_PATH "findable-partials-100-xqp.saved",
        INPUT_PATH "findable-partials-100-xad.saved",
        INPUT_PATH "findable-partials-100-xoo.saved",
        INPUT_PATH "findable-partials-100-xzagt.saved",
        INPUT_PATH "findable-partials-100-xzaec.saved",
        INPUT_PATH "findable-partials-100-xum.saved",
        INPUT_PATH "findable-partials-100-xzabe.saved",
        INPUT_PATH "findable-partials-100-xko.saved",
        INPUT_PATH "findable-partials-100-xzaal.saved",
        INPUT_PATH "findable-partials-100-xzaad.saved",
        INPUT_PATH "findable-partials-100-xzabz.saved",
        INPUT_PATH "findable-partials-100-xav.saved",
        INPUT_PATH "findable-partials-100-xjm.saved",
        INPUT_PATH "findable-partials-100-xqq.saved",
        INPUT_PATH "findable-partials-100-xqn.saved",
        INPUT_PATH "findable-partials-100-xqc.saved",
        INPUT_PATH "findable-partials-100-xzabs.saved",
        INPUT_PATH "findable-partials-100-xxa.saved",
        INPUT_PATH "findable-partials-100-xzagc.saved",
        INPUT_PATH "findable-partials-100-xzaco.saved",
        INPUT_PATH "findable-partials-100-xzafe.saved",
        INPUT_PATH "findable-partials-100-xbv.saved",
        INPUT_PATH "findable-partials-100-xzacr.saved",
        INPUT_PATH "findable-partials-100-xde.saved",
        INPUT_PATH "findable-partials-100-xsw.saved",
        INPUT_PATH "findable-partials-100-xzaen.saved",
        INPUT_PATH "findable-partials-100-xee.saved",
        INPUT_PATH "findable-partials-100-xkj.saved",
        INPUT_PATH "findable-partials-100-xni.saved",
        INPUT_PATH "findable-partials-100-xzafl.saved",
        INPUT_PATH "findable-partials-100-xif.saved",
        INPUT_PATH "findable-partials-100-xrw.saved",
        INPUT_PATH "findable-partials-100-xzaiy.saved",
        INPUT_PATH "findable-partials-100-xzabc.saved",
        INPUT_PATH "findable-partials-100-xhk.saved",
        INPUT_PATH "findable-partials-100-xzagi.saved",
        INPUT_PATH "findable-partials-100-xzacp.saved",
        INPUT_PATH "findable-partials-100-xoe.saved",
        INPUT_PATH "findable-partials-100-xms.saved",
        INPUT_PATH "findable-partials-100-xik.saved",
        INPUT_PATH "findable-partials-100-xzagh.saved",
        INPUT_PATH "findable-partials-100-xjj.saved",
        INPUT_PATH "findable-partials-100-xmp.saved",
        INPUT_PATH "findable-partials-100-xld.saved",
        INPUT_PATH "findable-partials-100-xwy.saved",
        INPUT_PATH "findable-partials-100-xzafs.saved",
        INPUT_PATH "findable-partials-100-xef.saved",
        INPUT_PATH "findable-partials-100-xzaeb.saved",
        INPUT_PATH "findable-partials-100-xdl.saved",
        INPUT_PATH "findable-partials-100-xrr.saved",
        INPUT_PATH "findable-partials-100-xqj.saved",
        INPUT_PATH "findable-partials-100-xps.saved",
        INPUT_PATH "findable-partials-100-xvj.saved",
        INPUT_PATH "findable-partials-100-xyg.saved",
        INPUT_PATH "findable-partials-100-xkt.saved",
        INPUT_PATH "findable-partials-100-xxg.saved",
        INPUT_PATH "findable-partials-100-xxj.saved",
        INPUT_PATH "findable-partials-100-xju.saved",
        INPUT_PATH "findable-partials-100-xjs.saved",
        INPUT_PATH "findable-partials-100-xsb.saved",
        INPUT_PATH "findable-partials-100-xhw.saved",
        INPUT_PATH "findable-partials-100-xzadv.saved",
        INPUT_PATH "findable-partials-100-xhe.saved",
        INPUT_PATH "findable-partials-100-xlm.saved",
        INPUT_PATH "findable-partials-100-xzadx.saved",
        INPUT_PATH "findable-partials-100-xii.saved",
        INPUT_PATH "findable-partials-100-xwr.saved",
        INPUT_PATH "findable-partials-100-xnk.saved",
        INPUT_PATH "findable-partials-100-xxk.saved",
        INPUT_PATH "findable-partials-100-xzaig.saved",
        INPUT_PATH "findable-partials-100-xlo.saved",
        INPUT_PATH "findable-partials-100-xzael.saved",
        INPUT_PATH "findable-partials-100-xaj.saved",
        INPUT_PATH "findable-partials-100-xen.saved",
        INPUT_PATH "findable-partials-100-xmz.saved",
        INPUT_PATH "findable-partials-100-xsg.saved",
        INPUT_PATH "findable-partials-100-xbo.saved",
        INPUT_PATH "findable-partials-100-xyj.saved",
        INPUT_PATH "findable-partials-100-xzacf.saved",
        INPUT_PATH "findable-partials-100-xrc.saved",
        INPUT_PATH "findable-partials-100-xhd.saved",
        INPUT_PATH "findable-partials-100-xrp.saved",
        INPUT_PATH "findable-partials-100-xat.saved",
        INPUT_PATH "findable-partials-100-xka.saved",
        INPUT_PATH "findable-partials-100-xma.saved",
        INPUT_PATH "findable-partials-100-xer.saved",
        INPUT_PATH "findable-partials-100-xss.saved",
        INPUT_PATH "findable-partials-100-xwg.saved",
        INPUT_PATH "findable-partials-100-xhc.saved",
        INPUT_PATH "findable-partials-100-xzabw.saved",
        INPUT_PATH "findable-partials-100-xfl.saved",
        INPUT_PATH "findable-partials-100-xok.saved",
        INPUT_PATH "findable-partials-100-xzagk.saved",
        INPUT_PATH "findable-partials-100-xtb.saved",
        INPUT_PATH "findable-partials-100-xoa.saved",
        INPUT_PATH "findable-partials-100-xwd.saved",
        INPUT_PATH "findable-partials-100-xij.saved",
        INPUT_PATH "findable-partials-100-xpl.saved",
        INPUT_PATH "findable-partials-100-xdu.saved",
        INPUT_PATH "findable-partials-100-xsa.saved",
        INPUT_PATH "findable-partials-100-xzacg.saved",
        INPUT_PATH "findable-partials-100-xel.saved",
        INPUT_PATH "findable-partials-100-xzafn.saved",
        INPUT_PATH "findable-partials-100-xoj.saved",
        INPUT_PATH "findable-partials-100-xzagu.saved",
        INPUT_PATH "findable-partials-100-xtl.saved",
        INPUT_PATH "findable-partials-100-xkz.saved",
        INPUT_PATH "findable-partials-100-xzais.saved",
        INPUT_PATH "findable-partials-100-xmo.saved",
        INPUT_PATH "findable-partials-100-xfz.saved",
        INPUT_PATH "findable-partials-100-xzaag.saved",
        INPUT_PATH "findable-partials-100-xzagd.saved",
        INPUT_PATH "findable-partials-100-xhp.saved",
        INPUT_PATH "findable-partials-100-xzach.saved",
        INPUT_PATH "findable-partials-100-xgv.saved",
        INPUT_PATH "findable-partials-100-xyw.saved",
        INPUT_PATH "findable-partials-100-xfe.saved",
        INPUT_PATH "findable-partials-100-xjv.saved",
        INPUT_PATH "findable-partials-100-xgp.saved",
        INPUT_PATH "findable-partials-100-xpg.saved",
        INPUT_PATH "findable-partials-100-xlk.saved",
        INPUT_PATH "findable-partials-100-xxt.saved",
        INPUT_PATH "findable-partials-100-xuy.saved",
        INPUT_PATH "findable-partials-100-xzade.saved",
        INPUT_PATH "findable-partials-100-xzagm.saved",
        INPUT_PATH "findable-partials-100-xpu.saved",
        INPUT_PATH "findable-partials-100-xrk.saved",
        INPUT_PATH "findable-partials-100-xzagw.saved",
        INPUT_PATH "findable-partials-100-xkw.saved",
        INPUT_PATH "findable-partials-100-xzahk.saved",
        INPUT_PATH "findable-partials-100-xwz.saved",
        INPUT_PATH "findable-partials-100-xww.saved",
        INPUT_PATH "findable-partials-100-xzacj.saved",
        INPUT_PATH "findable-partials-100-xuj.saved",
        INPUT_PATH "findable-partials-100-xzaej.saved",
        INPUT_PATH "findable-partials-100-xcc.saved",
        INPUT_PATH "findable-partials-100-xst.saved",
        INPUT_PATH "findable-partials-100-xxe.saved",
        INPUT_PATH "findable-partials-100-xun.saved",
        INPUT_PATH "findable-partials-100-xzafq.saved",
        INPUT_PATH "findable-partials-100-xzaht.saved",
        INPUT_PATH "findable-partials-100-xqs.saved",
        INPUT_PATH "findable-partials-100-xno.saved",
        INPUT_PATH "findable-partials-100-xkm.saved",
        INPUT_PATH "findable-partials-100-xzacw.saved",
        INPUT_PATH "findable-partials-100-xzahv.saved",
        INPUT_PATH "findable-partials-100-xzadl.saved",
        INPUT_PATH "findable-partials-100-xxf.saved",
        INPUT_PATH "findable-partials-100-xhi.saved",
        INPUT_PATH "findable-partials-100-xqo.saved",
        INPUT_PATH "findable-partials-100-xls.saved",
        INPUT_PATH "findable-partials-100-xzagb.saved",
        INPUT_PATH "findable-partials-100-xzafc.saved",
        INPUT_PATH "findable-partials-100-xiz.saved",
        INPUT_PATH "findable-partials-100-xub.saved",
        INPUT_PATH "findable-partials-100-xro.saved",
        INPUT_PATH "findable-partials-100-xip.saved",
        INPUT_PATH "findable-partials-100-xtx.saved",
        INPUT_PATH "findable-partials-100-xzacd.saved",
        INPUT_PATH "findable-partials-100-xzahw.saved",
        INPUT_PATH "findable-partials-100-xbm.saved",
        INPUT_PATH "findable-partials-100-xzacq.saved",
        INPUT_PATH "findable-partials-100-xpc.saved",
        INPUT_PATH "findable-partials-100-xtu.saved",
        INPUT_PATH "findable-partials-100-xzafo.saved",
        INPUT_PATH "findable-partials-100-xzafy.saved",
        INPUT_PATH "findable-partials-100-xxp.saved",
        INPUT_PATH "findable-partials-100-xeb.saved",
        INPUT_PATH "findable-partials-100-xzabx.saved",
        INPUT_PATH "findable-partials-100-xzadn.saved",
        INPUT_PATH "findable-partials-100-xxi.saved",
        INPUT_PATH "findable-partials-100-xvo.saved",
        INPUT_PATH "findable-partials-100-xyn.saved",
        INPUT_PATH "findable-partials-100-xfr.saved",
        INPUT_PATH "findable-partials-100-xvy.saved",
        INPUT_PATH "findable-partials-100-xod.saved",
        INPUT_PATH "findable-partials-100-xiu.saved",
        INPUT_PATH "findable-partials-100-xch.saved",
        INPUT_PATH "findable-partials-100-xzabk.saved",
        INPUT_PATH "findable-partials-100-xdr.saved",
        INPUT_PATH "findable-partials-100-xzact.saved",
        INPUT_PATH "findable-partials-100-xft.saved",
        INPUT_PATH "findable-partials-100-xvd.saved",
        INPUT_PATH "findable-partials-100-xwh.saved",
        INPUT_PATH "findable-partials-100-xzail.saved",
        INPUT_PATH "findable-partials-100-xxy.saved",
        INPUT_PATH "findable-partials-100-xyk.saved",
        INPUT_PATH "findable-partials-100-xmn.saved",
        INPUT_PATH "findable-partials-100-xfo.saved",
        INPUT_PATH "findable-partials-100-xqr.saved",
        INPUT_PATH "findable-partials-100-xzadi.saved",
        INPUT_PATH "findable-partials-100-xoy.saved",
        INPUT_PATH "findable-partials-100-xzaae.saved",
        INPUT_PATH "findable-partials-100-xbt.saved",
        INPUT_PATH "findable-partials-100-xzads.saved",
        INPUT_PATH "findable-partials-100-xzaiz.saved",
        INPUT_PATH "findable-partials-100-xan.saved",
        INPUT_PATH "findable-partials-100-xso.saved",
        INPUT_PATH "findable-partials-100-xei.saved",
        INPUT_PATH "findable-partials-100-xyd.saved",
        INPUT_PATH "findable-partials-100-xgz.saved",
        INPUT_PATH "findable-partials-100-xfq.saved",
        INPUT_PATH "findable-partials-100-xzaih.saved",
        INPUT_PATH "findable-partials-100-xwc.saved",
        INPUT_PATH "findable-partials-100-xes.saved",
        INPUT_PATH "findable-partials-100-xzacl.saved",
        INPUT_PATH "findable-partials-100-xbx.saved",
        INPUT_PATH "findable-partials-100-xjw.saved",
        INPUT_PATH "findable-partials-100-xzacc.saved",
        INPUT_PATH "findable-partials-100-xex.saved",
        INPUT_PATH "findable-partials-100-xon.saved",
        INPUT_PATH "findable-partials-100-xvw.saved",
        INPUT_PATH "findable-partials-100-xwe.saved",
        INPUT_PATH "findable-partials-100-xfd.saved",
        INPUT_PATH "findable-partials-100-xvi.saved",
        INPUT_PATH "findable-partials-100-xkk.saved",
        INPUT_PATH "findable-partials-100-xzaaa.saved",
        INPUT_PATH "findable-partials-100-xzagl.saved",
        INPUT_PATH "findable-partials-100-xfc.saved",
        INPUT_PATH "findable-partials-100-xzahl.saved",
        INPUT_PATH "findable-partials-100-xev.saved",
        INPUT_PATH "findable-partials-100-xdy.saved",
        INPUT_PATH "findable-partials-100-xbz.saved",
        INPUT_PATH "findable-partials-100-xsu.saved",
        INPUT_PATH "findable-partials-100-xzaah.saved",
        INPUT_PATH "findable-partials-100-xgh.saved",
        INPUT_PATH "findable-partials-100-xmt.saved",
        INPUT_PATH "findable-partials-100-xty.saved",
        INPUT_PATH "findable-partials-100-xzahp.saved",
        INPUT_PATH "findable-partials-100-xia.saved",
        INPUT_PATH "findable-partials-100-xsc.saved",
        INPUT_PATH "findable-partials-100-xnd.saved",
        INPUT_PATH "findable-partials-100-xeu.saved",
        INPUT_PATH "findable-partials-100-xzaeq.saved",
        INPUT_PATH "findable-partials-100-xpa.saved",
        INPUT_PATH "findable-partials-100-xzaer.saved",
        INPUT_PATH "findable-partials-100-xuf.saved",
        INPUT_PATH "findable-partials-100-xsq.saved",
        INPUT_PATH "findable-partials-100-xuo.saved",
        INPUT_PATH "findable-partials-100-xbf.saved",
        INPUT_PATH "findable-partials-100-xdp.saved",
        INPUT_PATH "findable-partials-100-xwa.saved",
        INPUT_PATH "findable-partials-100-xzaim.saved",
        INPUT_PATH "findable-partials-100-xtp.saved",
        INPUT_PATH "findable-partials-100-xri.saved",
        INPUT_PATH "findable-partials-100-xzaaj.saved",
        INPUT_PATH "findable-partials-100-xzahe.saved",
        INPUT_PATH "findable-partials-100-xhh.saved",
        INPUT_PATH "findable-partials-100-xra.saved",
        INPUT_PATH "findable-partials-100-xzagr.saved",
        INPUT_PATH "findable-partials-100-xud.saved",
        INPUT_PATH "findable-partials-100-xry.saved",
        INPUT_PATH "findable-partials-100-xgg.saved",
        INPUT_PATH "findable-partials-100-xzaff.saved",
        INPUT_PATH "findable-partials-100-xsm.saved",
        INPUT_PATH "findable-partials-100-xkd.saved",
        INPUT_PATH "findable-partials-100-xgm.saved",
        INPUT_PATH "findable-partials-100-xiv.saved",
        INPUT_PATH "findable-partials-100-xfa.saved",
        INPUT_PATH "findable-partials-100-xlq.saved",
        INPUT_PATH "findable-partials-100-xlc.saved",
        INPUT_PATH "findable-partials-100-xvc.saved",
        INPUT_PATH "findable-partials-100-xhq.saved",
        INPUT_PATH "findable-partials-100-xlu.saved",
        INPUT_PATH "findable-partials-100-xbw.saved",
        INPUT_PATH "findable-partials-100-xya.saved",
        INPUT_PATH "findable-partials-100-xzafz.saved",
        INPUT_PATH "findable-partials-100-xzago.saved",
        INPUT_PATH "findable-partials-100-xhn.saved",
        INPUT_PATH "findable-partials-100-xwt.saved",
        INPUT_PATH "findable-partials-100-xkv.saved",
        INPUT_PATH "findable-partials-100-xkx.saved",
        INPUT_PATH "findable-partials-100-xfu.saved",
        INPUT_PATH "findable-partials-100-xau.saved",
        INPUT_PATH "findable-partials-100-xzaaw.saved",
        INPUT_PATH "findable-partials-100-xjn.saved",
        INPUT_PATH "findable-partials-100-xzaat.saved",
        INPUT_PATH "findable-partials-100-xzabm.saved",
        INPUT_PATH "findable-partials-100-xcu.saved",
        INPUT_PATH "findable-partials-100-xmw.saved",
        INPUT_PATH "findable-partials-100-xjd.saved",
        INPUT_PATH "findable-partials-100-xnm.saved",
        INPUT_PATH "findable-partials-100-xzaio.saved",
        INPUT_PATH "findable-partials-100-xab.saved",
        INPUT_PATH "findable-partials-100-xzafr.saved",
        INPUT_PATH "findable-partials-100-xoi.saved",
        INPUT_PATH "findable-partials-100-xtz.saved",
        INPUT_PATH "findable-partials-100-xur.saved",
        INPUT_PATH "findable-partials-100-xeg.saved",
        INPUT_PATH "findable-partials-100-xnp.saved",
        INPUT_PATH "findable-partials-100-xdw.saved",
        INPUT_PATH "findable-partials-100-xom.saved",
        INPUT_PATH "findable-partials-100-xzadj.saved",
        INPUT_PATH "findable-partials-100-xzaie.saved",
        INPUT_PATH "findable-partials-100-xpz.saved",
        INPUT_PATH "findable-partials-100-xyc.saved",
        INPUT_PATH "findable-partials-100-xao.saved",
        INPUT_PATH "findable-partials-100-xcz.saved",
        INPUT_PATH "findable-partials-100-xtf.saved",
        INPUT_PATH "findable-partials-100-xlv.saved",
        INPUT_PATH "findable-partials-100-xog.saved",
        INPUT_PATH "findable-partials-100-xys.saved",
        INPUT_PATH "findable-partials-100-xmv.saved",
        INPUT_PATH "findable-partials-100-xsv.saved",
        INPUT_PATH "findable-partials-100-xbb.saved",
        INPUT_PATH "findable-partials-100-xjf.saved",
        INPUT_PATH "findable-partials-100-xig.saved",
        INPUT_PATH "findable-partials-100-xnl.saved",
        INPUT_PATH "findable-partials-100-xzaab.saved",
        INPUT_PATH "findable-partials-100-xzaix.saved",
        INPUT_PATH "findable-partials-100-xnj.saved",
        INPUT_PATH "findable-partials-100-xmb.saved",
        INPUT_PATH "findable-partials-100-xvp.saved",
        INPUT_PATH "findable-partials-100-xdh.saved",
        INPUT_PATH "findable-partials-100-xar.saved",
        INPUT_PATH "findable-partials-100-xru.saved",
        INPUT_PATH "findable-partials-100-xeo.saved",
        INPUT_PATH "findable-partials-100-xqu.saved",
        INPUT_PATH "findable-partials-100-xzaao.saved",
        INPUT_PATH "findable-partials-100-xtk.saved",
        INPUT_PATH "findable-partials-100-xpr.saved",
        INPUT_PATH "findable-partials-100-xli.saved",
        INPUT_PATH "findable-partials-100-xuc.saved",
        INPUT_PATH "findable-partials-100-xwv.saved",
        INPUT_PATH "findable-partials-100-xaf.saved",
        INPUT_PATH "findable-partials-100-xgw.saved",
        INPUT_PATH "findable-partials-100-xzadm.saved",
        INPUT_PATH "findable-partials-100-xfp.saved",
        INPUT_PATH "findable-partials-100-xnt.saved",
        INPUT_PATH "findable-partials-100-xzaez.saved",
        INPUT_PATH "findable-partials-100-xpp.saved",
        INPUT_PATH "findable-partials-100-xzabq.saved",
        INPUT_PATH "findable-partials-100-xlt.saved",
        INPUT_PATH "findable-partials-100-xqt.saved",
        INPUT_PATH "findable-partials-100-xzaba.saved",
        INPUT_PATH "findable-partials-100-xhm.saved",
        INPUT_PATH "findable-partials-100-xrm.saved",
        INPUT_PATH "findable-partials-100-xmg.saved",
        INPUT_PATH "findable-partials-100-xnx.saved",
        INPUT_PATH "findable-partials-100-xje.saved",
        INPUT_PATH "findable-partials-100-xba.saved",
        INPUT_PATH "findable-partials-100-xqk.saved",
        INPUT_PATH "findable-partials-100-xfh.saved",
        INPUT_PATH "findable-partials-100-xzaet.saved",
        INPUT_PATH "findable-partials-100-xvx.saved",
        INPUT_PATH "findable-partials-100-xvh.saved",
        INPUT_PATH "findable-partials-100-xyy.saved",
        INPUT_PATH "findable-partials-100-xpq.saved",
        INPUT_PATH "findable-partials-100-xja.saved",
        INPUT_PATH "findable-partials-100-xsx.saved",
        INPUT_PATH "findable-partials-100-xgj.saved",
        INPUT_PATH "findable-partials-100-xox.saved",
        INPUT_PATH "findable-partials-100-xtd.saved",
        INPUT_PATH "findable-partials-100-xvs.saved",
        INPUT_PATH "findable-partials-100-xzadf.saved",
        INPUT_PATH "findable-partials-100-xmi.saved",
        INPUT_PATH "findable-partials-100-xcq.saved",
        INPUT_PATH "findable-partials-100-xaw.saved",
        INPUT_PATH "findable-partials-100-xsf.saved",
        INPUT_PATH "findable-partials-100-xzaem.saved",
        INPUT_PATH "findable-partials-100-xlg.saved",
        INPUT_PATH "findable-partials-100-xzaes.saved",
        INPUT_PATH "findable-partials-100-xgt.saved",
        INPUT_PATH "findable-partials-100-xzadg.saved",
        INPUT_PATH "findable-partials-100-xqz.saved",
        INPUT_PATH "findable-partials-100-xhv.saved",
        INPUT_PATH "findable-partials-100-xej.saved",
        INPUT_PATH "findable-partials-100-xio.saved",
        INPUT_PATH "findable-partials-100-xzafk.saved",
        INPUT_PATH "findable-partials-100-xzaan.saved",
        INPUT_PATH "findable-partials-100-xuv.saved",
        INPUT_PATH "findable-partials-100-xzaiv.saved",
        INPUT_PATH "findable-partials-100-xdo.saved",
        INPUT_PATH "findable-partials-100-xsp.saved",
        INPUT_PATH "findable-partials-100-xff.saved",
        INPUT_PATH "findable-partials-100-xzafg.saved",
        INPUT_PATH "findable-partials-100-xdt.saved",
        INPUT_PATH "findable-partials-100-xzabp.saved",
        INPUT_PATH "findable-partials-100-xuu.saved",
        INPUT_PATH "findable-partials-100-xzafa.saved",
        INPUT_PATH "findable-partials-100-xbu.saved",
        INPUT_PATH "findable-partials-100-xji.saved",
        INPUT_PATH "findable-partials-100-xwl.saved",
        INPUT_PATH "findable-partials-100-xbl.saved",
        INPUT_PATH "findable-partials-100-xzahf.saved",
        INPUT_PATH "findable-partials-100-xsj.saved",
        INPUT_PATH "findable-partials-100-xvq.saved",
        INPUT_PATH "findable-partials-100-xyo.saved",
        INPUT_PATH "findable-partials-100-xib.saved",
        INPUT_PATH "findable-partials-100-xag.saved",
        INPUT_PATH "findable-partials-100-xgu.saved",
        INPUT_PATH "findable-partials-100-xbi.saved",
        INPUT_PATH "findable-partials-100-xrq.saved",
        INPUT_PATH "findable-partials-100-xyq.saved",
        INPUT_PATH "findable-partials-100-xbd.saved",
        INPUT_PATH "findable-partials-100-xln.saved",
        INPUT_PATH "findable-partials-100-xmm.saved",
        INPUT_PATH "findable-partials-100-xna.saved",
        INPUT_PATH "findable-partials-100-xyb.saved",
        INPUT_PATH "findable-partials-100-xkb.saved",
        INPUT_PATH "findable-partials-100-xou.saved",
        INPUT_PATH "findable-partials-100-xew.saved",
        INPUT_PATH "findable-partials-100-xiy.saved",
        INPUT_PATH "findable-partials-100-xxs.saved",
        INPUT_PATH "findable-partials-100-xcf.saved",
        INPUT_PATH "findable-partials-100-xzaee.saved",
        INPUT_PATH "findable-partials-100-xby.saved",
        INPUT_PATH "findable-partials-100-xih.saved",
        INPUT_PATH "findable-partials-100-xnh.saved",
        INPUT_PATH "findable-partials-100-xzabn.saved",
        INPUT_PATH "findable-partials-100-xue.saved",
        INPUT_PATH "findable-partials-100-xuz.saved",
        INPUT_PATH "findable-partials-100-xzaif.saved",
        INPUT_PATH "findable-partials-100-xvv.saved",
        INPUT_PATH "findable-partials-100-xol.saved",
        INPUT_PATH "findable-partials-100-xzahd.saved",
        INPUT_PATH "findable-partials-100-xul.saved",
        INPUT_PATH "findable-partials-100-xgo.saved",
        INPUT_PATH "findable-partials-100-xmr.saved",
        INPUT_PATH "findable-partials-100-xpw.saved",
        INPUT_PATH "findable-partials-100-xyt.saved",
        INPUT_PATH "findable-partials-100-xaz.saved",
        INPUT_PATH "findable-partials-100-xil.saved",
        INPUT_PATH "findable-partials-100-xzaaq.saved",
        INPUT_PATH "findable-partials-100-xla.saved",
        INPUT_PATH "findable-partials-100-xzadk.saved",
        INPUT_PATH "findable-partials-100-xnu.saved",
        INPUT_PATH "findable-partials-100-xvn.saved",
        INPUT_PATH "findable-partials-100-xuk.saved",
        INPUT_PATH "findable-partials-100-xzaaf.saved",
        INPUT_PATH "findable-partials-100-xne.saved",
        INPUT_PATH "findable-partials-100-xzaei.saved",
        INPUT_PATH "findable-partials-100-xkc.saved",
        INPUT_PATH "findable-partials-100-xua.saved",
        INPUT_PATH "findable-partials-100-xly.saved",
        INPUT_PATH "findable-partials-100-xrx.saved",
        INPUT_PATH "findable-partials-100-xzafp.saved",
        INPUT_PATH "findable-partials-100-xsn.saved",
        INPUT_PATH "findable-partials-100-xax.saved",
        INPUT_PATH "findable-partials-100-xzair.saved",
        INPUT_PATH "findable-partials-100-xyx.saved",
        INPUT_PATH "findable-partials-100-xpo.saved",
        INPUT_PATH "findable-partials-100-xqm.saved",
        INPUT_PATH "findable-partials-100-xzafb.saved",
        INPUT_PATH "findable-partials-100-xzaav.saved",
        INPUT_PATH "findable-partials-100-xoq.saved",
        INPUT_PATH "findable-partials-100-xin.saved",
        INPUT_PATH "findable-partials-100-xjg.saved",
        INPUT_PATH "findable-partials-100-xqw.saved",
        INPUT_PATH "findable-partials-100-xvm.saved",
        INPUT_PATH "findable-partials-100-xzahr.saved",
        INPUT_PATH "findable-partials-100-xid.saved",
        INPUT_PATH "findable-partials-100-xzafj.saved",
        INPUT_PATH "findable-partials-100-xah.saved",
        INPUT_PATH "findable-partials-100-xhy.saved",
        INPUT_PATH "findable-partials-100-xzabu.saved",
        INPUT_PATH "findable-partials-100-xdz.saved",
        INPUT_PATH "findable-partials-100-xtm.saved",
        INPUT_PATH "findable-partials-100-xxl.saved",
        INPUT_PATH "findable-partials-100-xgb.saved",
        INPUT_PATH "findable-partials-100-xgy.saved",
        INPUT_PATH "findable-partials-100-xtr.saved",
        INPUT_PATH "findable-partials-100-xsh.saved",
        INPUT_PATH "findable-partials-100-xwx.saved",
        INPUT_PATH "findable-partials-100-xlw.saved",
        INPUT_PATH "findable-partials-100-xhl.saved",
        INPUT_PATH "findable-partials-100-xvb.saved",
        INPUT_PATH "findable-partials-100-xzady.saved",
        INPUT_PATH "findable-partials-100-xeh.saved",
        INPUT_PATH "findable-partials-100-xuw.saved",
        INPUT_PATH "findable-partials-100-xwm.saved",
        INPUT_PATH "findable-partials-100-xvl.saved",
        INPUT_PATH "findable-partials-100-xcd.saved",
        INPUT_PATH "findable-partials-100-xxb.saved",
        INPUT_PATH "findable-partials-100-xzacb.saved",
        INPUT_PATH "findable-partials-100-xmy.saved",
        INPUT_PATH "findable-partials-100-xbp.saved",
        INPUT_PATH "findable-partials-100-xnc.saved",
        INPUT_PATH "findable-partials-100-xge.saved",
        INPUT_PATH "findable-partials-100-xzaay.saved",
        INPUT_PATH "findable-partials-100-xzabo.saved",
        INPUT_PATH "findable-partials-100-xfj.saved",
        INPUT_PATH "findable-partials-100-xzaev.saved",
        INPUT_PATH "findable-partials-100-xqb.saved",
        INPUT_PATH "findable-partials-100-xyi.saved",
        INPUT_PATH "findable-partials-100-xsy.saved",
        INPUT_PATH "findable-partials-100-xzahx.saved",
        INPUT_PATH "findable-partials-100-xzaey.saved",
        INPUT_PATH "findable-partials-100-xzabh.saved",
        INPUT_PATH "findable-partials-100-xwi.saved",
        INPUT_PATH "findable-partials-100-xzaci.saved",
        INPUT_PATH "findable-partials-100-xus.saved",
        INPUT_PATH "findable-partials-100-xxw.saved",
        INPUT_PATH "findable-partials-100-xzabb.saved",
        INPUT_PATH "findable-partials-100-xzaby.saved",
        INPUT_PATH "findable-partials-100-xkg.saved",
        INPUT_PATH "findable-partials-100-xzaiw.saved",
        INPUT_PATH "findable-partials-100-xbn.saved",
        INPUT_PATH "findable-partials-100-xym.saved",
        INPUT_PATH "findable-partials-100-xrt.saved",
        INPUT_PATH "findable-partials-100-xmu.saved",
        INPUT_PATH "findable-partials-100-xcy.saved",
        INPUT_PATH "findable-partials-100-xof.saved",
        INPUT_PATH "findable-partials-100-xjz.saved",
        INPUT_PATH "findable-partials-100-xaa.saved",
        INPUT_PATH "findable-partials-100-xqd.saved",
        INPUT_PATH "findable-partials-100-xlf.saved",
        INPUT_PATH "findable-partials-100-xxz.saved",
        INPUT_PATH "findable-partials-100-xfm.saved",
        INPUT_PATH "findable-partials-100-xrh.saved",
        INPUT_PATH "findable-partials-100-xot.saved",
        INPUT_PATH "findable-partials-100-xaq.saved",
        INPUT_PATH "findable-partials-100-xqi.saved",
        INPUT_PATH "findable-partials-100-xnb.saved",
        INPUT_PATH "findable-partials-100-xbh.saved",
        INPUT_PATH "findable-partials-100-xap.saved",
        INPUT_PATH "findable-partials-100-xbc.saved",
        INPUT_PATH "findable-partials-100-xkl.saved",
        INPUT_PATH "findable-partials-100-xcr.saved",
        INPUT_PATH "findable-partials-100-xdv.saved",
        INPUT_PATH "findable-partials-100-xpy.saved",
        INPUT_PATH "findable-partials-100-xts.saved",
        INPUT_PATH "findable-partials-100-xki.saved",
        INPUT_PATH "findable-partials-100-xiw.saved",
        INPUT_PATH "findable-partials-100-xzaac.saved",
        INPUT_PATH "findable-partials-100-xzaep.saved",
        INPUT_PATH "findable-partials-100-xpe.saved",
        INPUT_PATH "findable-partials-100-xmj.saved",
        INPUT_PATH "findable-partials-100-xzaar.saved",
        INPUT_PATH "findable-partials-100-xzacs.saved",
        INPUT_PATH "findable-partials-100-xdn.saved",
        INPUT_PATH "findable-partials-100-xzage.saved",
        INPUT_PATH "findable-partials-100-xca.saved",
        INPUT_PATH "findable-partials-100-xtw.saved",
        INPUT_PATH "findable-partials-100-xac.saved",
        INPUT_PATH "findable-partials-100-xqh.saved",
        INPUT_PATH "findable-partials-100-xjy.saved",
        INPUT_PATH "findable-partials-100-xey.saved",
        INPUT_PATH "findable-partials-100-xpj.saved",
        INPUT_PATH "findable-partials-100-xzado.saved",
        INPUT_PATH "findable-partials-100-xkf.saved",
        INPUT_PATH "findable-partials-100-xfv.saved",
        INPUT_PATH "findable-partials-100-xzacu.saved",
        INPUT_PATH "findable-partials-100-xzaau.saved",
        INPUT_PATH "findable-partials-100-xug.saved",
        INPUT_PATH "findable-partials-100-xkh.saved",
        INPUT_PATH "findable-partials-100-xow.saved",
        INPUT_PATH "findable-partials-100-xfn.saved",
        INPUT_PATH "findable-partials-100-xxn.saved",
        INPUT_PATH "findable-partials-100-xzags.saved",
        INPUT_PATH "findable-partials-100-xzafv.saved",
        INPUT_PATH "findable-partials-100-xmc.saved",
        INPUT_PATH "findable-partials-100-xzabi.saved",
        INPUT_PATH "findable-partials-100-xup.saved",
        INPUT_PATH "findable-partials-100-xzadp.saved",
        INPUT_PATH "findable-partials-100-xfg.saved",
        INPUT_PATH "findable-partials-100-xpn.saved",
        INPUT_PATH "findable-partials-100-xhr.saved",
        INPUT_PATH "findable-partials-100-xov.saved",
        INPUT_PATH "findable-partials-100-xny.saved",
        INPUT_PATH "findable-partials-100-xwq.saved",
        INPUT_PATH "findable-partials-100-xzacn.saved",
        INPUT_PATH "findable-partials-100-xtj.saved",
        INPUT_PATH "findable-partials-100-xjk.saved",
        INPUT_PATH "findable-partials-100-xdc.saved",
        INPUT_PATH "findable-partials-100-xbs.saved",
        INPUT_PATH "findable-partials-100-xea.saved",
        INPUT_PATH "findable-partials-100-xjl.saved",
        INPUT_PATH "findable-partials-100-xix.saved",
        INPUT_PATH "findable-partials-100-xdk.saved",
        INPUT_PATH "findable-partials-100-xco.saved",
        INPUT_PATH "findable-partials-100-xhu.saved",
        INPUT_PATH "findable-partials-100-xrd.saved",
        INPUT_PATH "findable-partials-100-xsk.saved",
        INPUT_PATH "findable-partials-100-xai.saved",
        INPUT_PATH "findable-partials-100-xkq.saved",
        INPUT_PATH "findable-partials-100-xte.saved",
        INPUT_PATH "findable-partials-100-xjc.saved",
        INPUT_PATH "findable-partials-100-xwo.saved",
        INPUT_PATH "findable-partials-100-xzaex.saved",
        INPUT_PATH "findable-partials-100-xjh.saved",
        INPUT_PATH "findable-partials-100-xzabt.saved",
        INPUT_PATH "findable-partials-100-xyl.saved",
        INPUT_PATH "findable-partials-100-xyf.saved",
        INPUT_PATH "findable-partials-100-xzaiq.saved",
        INPUT_PATH "findable-partials-100-xfx.saved",
        INPUT_PATH "findable-partials-100-xlh.saved",
        INPUT_PATH "findable-partials-100-xqa.saved",
        INPUT_PATH "findable-partials-100-xzafx.saved",
        INPUT_PATH "findable-partials-100-xdm.saved",
        INPUT_PATH "findable-partials-100-xzabl.saved",
        INPUT_PATH "findable-partials-100-xzace.saved",
        INPUT_PATH "findable-partials-100-xci.saved",
        INPUT_PATH "findable-partials-100-xzaew.saved",
        INPUT_PATH "findable-partials-100-xzaho.saved",
        INPUT_PATH "findable-partials-100-xzaeu.saved",
        INPUT_PATH "findable-partials-100-xsd.saved",
        INPUT_PATH "findable-partials-100-xvg.saved",
        INPUT_PATH "findable-partials-100-xtg.saved",
        INPUT_PATH "findable-partials-100-xzagz.saved",
        INPUT_PATH "findable-partials-100-xgd.saved",
        INPUT_PATH "findable-partials-100-xdf.saved",
        INPUT_PATH "findable-partials-100-xtv.saved",
        INPUT_PATH "findable-partials-100-xut.saved",
        INPUT_PATH "findable-partials-100-xzafh.saved",
        INPUT_PATH "findable-partials-100-xpk.saved",
        INPUT_PATH "findable-partials-100-xzacy.saved",
        INPUT_PATH "findable-partials-100-xnr.saved",
        INPUT_PATH "findable-partials-100-xsl.saved",
        INPUT_PATH "findable-partials-100-xdi.saved",
        INPUT_PATH "findable-partials-100-xhg.saved",
        INPUT_PATH "findable-partials-100-xbk.saved",
        INPUT_PATH "findable-partials-100-xzadd.saved",
        INPUT_PATH "findable-partials-100-xvu.saved",
        INPUT_PATH "findable-partials-100-xzafw.saved",
        INPUT_PATH "findable-partials-100-xzadq.saved",
        INPUT_PATH "findable-partials-100-xzaea.saved",
        INPUT_PATH "findable-partials-100-xzahy.saved",
        INPUT_PATH "findable-partials-100-xzaeh.saved",
        INPUT_PATH "findable-partials-100-xet.saved",
        INPUT_PATH "findable-partials-100-xpv.saved",
        INPUT_PATH "findable-partials-100-xfy.saved",
        INPUT_PATH "findable-partials-100-xzagq.saved",
        INPUT_PATH "findable-partials-100-xth.saved",
        INPUT_PATH "findable-partials-100-xzahc.saved",
        INPUT_PATH "findable-partials-100-xlp.saved",
        INPUT_PATH "findable-partials-100-xit.saved",
        INPUT_PATH "findable-partials-100-xzadc.saved",
        INPUT_PATH "findable-partials-100-xgk.saved",
        INPUT_PATH "findable-partials-100-xyp.saved",
        INPUT_PATH "findable-partials-100-xmh.saved",
        INPUT_PATH "findable-partials-100-xck.saved",
        INPUT_PATH "findable-partials-100-xke.saved",
        INPUT_PATH "findable-partials-100-xzahb.saved",
        INPUT_PATH "findable-partials-100-xzahi.saved",
        INPUT_PATH "findable-partials-100-xwj.saved",
        INPUT_PATH "findable-partials-100-xda.saved",
        INPUT_PATH "findable-partials-100-xfs.saved",
        INPUT_PATH "findable-partials-100-xgq.saved",
        INPUT_PATH "findable-partials-100-xhz.saved",
        INPUT_PATH "findable-partials-100-xwu.saved",
        INPUT_PATH "findable-partials-100-xnf.saved",
        INPUT_PATH "findable-partials-100-xzabr.saved",
        INPUT_PATH "findable-partials-100-xre.saved",
        INPUT_PATH "findable-partials-100-xcs.saved",
        INPUT_PATH "findable-partials-100-xzahh.saved",
        INPUT_PATH "findable-partials-100-xzagf.saved",
        INPUT_PATH "findable-partials-100-xrl.saved",
        INPUT_PATH "findable-partials-100-xhb.saved",
        INPUT_PATH "findable-partials-100-xjp.saved",
        INPUT_PATH "findable-partials-100-xjt.saved",
        INPUT_PATH "findable-partials-100-xzaed.saved",
        INPUT_PATH "findable-partials-100-xzahn.saved",
        INPUT_PATH "findable-partials-100-xuh.saved",
        INPUT_PATH "findable-partials-100-xzadt.saved",
        INPUT_PATH "findable-partials-100-xti.saved",
        INPUT_PATH "findable-partials-100-xgn.saved",
        INPUT_PATH "findable-partials-100-xvr.saved",
        INPUT_PATH "findable-partials-100-xzaas.saved",
        INPUT_PATH "findable-partials-100-xcb.saved",
        INPUT_PATH "findable-partials-100-xxc.saved",
        INPUT_PATH "findable-partials-100-xsr.saved",
        INPUT_PATH "findable-partials-100-xme.saved",
        INPUT_PATH "findable-partials-100-xye.saved",
        INPUT_PATH "findable-partials-100-xyu.saved",
        INPUT_PATH "findable-partials-100-xux.saved",
        INPUT_PATH "findable-partials-100-xse.saved",
        INPUT_PATH "findable-partials-100-xec.saved",
        INPUT_PATH "findable-partials-100-xzagn.saved",
        INPUT_PATH "findable-partials-100-xzabj.saved",
        INPUT_PATH "findable-partials-100-xzaeg.saved",
        INPUT_PATH "findable-partials-100-xbe.saved",
        INPUT_PATH "findable-partials-100-xzada.saved",
        INPUT_PATH "findable-partials-100-xdq.saved",
        INPUT_PATH "findable-partials-100-xay.saved",
        INPUT_PATH "findable-partials-100-xem.saved",
        INPUT_PATH "findable-partials-100-xzaam.saved",
        INPUT_PATH "findable-partials-100-xoc.saved",
        INPUT_PATH "findable-partials-100-xcl.saved",
        INPUT_PATH "findable-partials-100-xgc.saved",
        INPUT_PATH "findable-partials-100-xho.saved",
        INPUT_PATH "findable-partials-100-xbg.saved",
        INPUT_PATH "findable-partials-100-xxo.saved",
        INPUT_PATH "findable-partials-100-xrj.saved",
        INPUT_PATH "findable-partials-100-xbr.saved",
        INPUT_PATH "findable-partials-100-xsi.saved",
        INPUT_PATH "findable-partials-100-xzacx.saved",
        INPUT_PATH "findable-partials-100-xqy.saved",
        INPUT_PATH "findable-partials-100-xph.saved",
        INPUT_PATH "findable-partials-100-xzacz.saved",
        INPUT_PATH "findable-partials-100-xfw.saved",
        INPUT_PATH "findable-partials-100-xpt.saved",
        INPUT_PATH "findable-partials-100-xvz.saved",
        INPUT_PATH "findable-partials-100-xhj.saved",
        INPUT_PATH "findable-partials-100-xzaiu.saved",
        INPUT_PATH "findable-partials-100-xjr.saved",
        INPUT_PATH "findable-partials-100-xhs.saved",
        INPUT_PATH "findable-partials-100-xrz.saved",
        INPUT_PATH "findable-partials-100-xzahg.saved",
        INPUT_PATH "findable-partials-100-xlx.saved",
        INPUT_PATH "findable-partials-100-xzabf.saved",
        INPUT_PATH "findable-partials-100-xzagp.saved",
        INPUT_PATH "findable-partials-100-xkr.saved",
        INPUT_PATH "findable-partials-100-xxq.saved",
        INPUT_PATH "findable-partials-100-xcw.saved",
        INPUT_PATH "findable-partials-100-xnq.saved",
        INPUT_PATH "findable-partials-100-xwf.saved",
        INPUT_PATH "findable-partials-100-xzaeo.saved",
        INPUT_PATH "findable-partials-100-xfk.saved",
        INPUT_PATH "findable-partials-100-xlr.saved",
        INPUT_PATH "findable-partials-100-xzagg.saved",
        INPUT_PATH "findable-partials-100-xdx.saved",
        INPUT_PATH "findable-partials-100-xfb.saved",
        INPUT_PATH "findable-partials-100-xcj.saved",
        INPUT_PATH "findable-partials-100-xnz.saved",
        INPUT_PATH "findable-partials-100-xxx.saved",
        INPUT_PATH "findable-partials-100-xrb.saved",
        INPUT_PATH "findable-partials-100-xqx.saved",
        INPUT_PATH "findable-partials-100-xzadu.saved",
        INPUT_PATH "findable-partials-100-xzahq.saved",
        INPUT_PATH "findable-partials-100-xpb.saved",
        INPUT_PATH "findable-partials-100-xpi.saved",
        INPUT_PATH "findable-partials-100-xky.saved",
        INPUT_PATH "findable-partials-100-xzaax.saved",
        INPUT_PATH "findable-partials-100-xlj.saved",
        INPUT_PATH "findable-partials-100-xqe.saved",
        INPUT_PATH "findable-partials-100-xdb.saved",
        INPUT_PATH "findable-partials-100-xoz.saved",
        INPUT_PATH "findable-partials-100-xas.saved",
        INPUT_PATH "findable-partials-100-xae.saved",
        INPUT_PATH "findable-partials-100-xop.saved",
        INPUT_PATH "findable-partials-100-xdg.saved",
        INPUT_PATH "findable-partials-100-xpx.saved",
        INPUT_PATH "findable-partials-100-xez.saved",
        INPUT_PATH "findable-partials-100-xha.saved",
        INPUT_PATH "findable-partials-100-xcp.saved",
        INPUT_PATH "findable-partials-100-xcx.saved",
        INPUT_PATH "findable-partials-100-xfi.saved",
        INPUT_PATH "findable-partials-100-xzacv.saved",
        INPUT_PATH "findable-partials-100-xmk.saved",
        INPUT_PATH "findable-partials-100-xmx.saved",
        INPUT_PATH "findable-partials-100-xzaek.saved",
        INPUT_PATH "findable-partials-100-xzafi.saved",
        INPUT_PATH "findable-partials-100-xto.saved",
        INPUT_PATH "findable-partials-100-xvf.saved",
        INPUT_PATH "findable-partials-100-xal.saved",
        INPUT_PATH "findable-partials-100-xhx.saved",
        INPUT_PATH "findable-partials-100-xzabd.saved",
        INPUT_PATH "findable-partials-100-xtc.saved",
        INPUT_PATH "findable-partials-100-xhf.saved",
        INPUT_PATH "findable-partials-100-xzahj.saved",
        INPUT_PATH "findable-partials-100-xzahz.saved",
        INPUT_PATH "findable-partials-100-xrv.saved",
        INPUT_PATH "findable-partials-100-xgr.saved",
        INPUT_PATH "findable-partials-100-xzagx.saved",
        INPUT_PATH "findable-partials-100-xeq.saved",
        INPUT_PATH "findable-partials-100-xzacm.saved",
        INPUT_PATH "findable-partials-100-xqf.saved",
        INPUT_PATH "findable-partials-100-xzait.saved",
        INPUT_PATH "findable-partials-100-xek.saved",
        INPUT_PATH "findable-partials-100-xxv.saved",
        INPUT_PATH "findable-partials-100-xku.saved",
        INPUT_PATH "findable-partials-100-xgs.saved",
        INPUT_PATH "findable-partials-100-xui.saved",
        INPUT_PATH "findable-partials-100-xrn.saved",
        INPUT_PATH "findable-partials-100-xyh.saved",
        INPUT_PATH "findable-partials-100-xyv.saved",
        INPUT_PATH "findable-partials-100-xim.saved",
        INPUT_PATH "findable-partials-100-xcg.saved",
        INPUT_PATH "findable-partials-100-xrg.saved",
        INPUT_PATH "findable-partials-100-xwk.saved",
        INPUT_PATH "findable-partials-100-xxu.saved",
        INPUT_PATH "findable-partials-100-xjx.saved",
        INPUT_PATH "findable-partials-100-xht.saved",
        INPUT_PATH "findable-partials-100-xic.saved",
        INPUT_PATH "findable-partials-100-xzaft.saved",
        INPUT_PATH "findable-partials-100-xxm.saved",
        INPUT_PATH "findable-partials-100-xwb.saved",
        INPUT_PATH "findable-partials-100-xzadb.saved",
        INPUT_PATH "findable-partials-100-xzabg.saved",
        INPUT_PATH "findable-partials-100-xed.saved",
        INPUT_PATH "findable-partials-100-xpm.saved",
        INPUT_PATH "findable-partials-100-xgf.saved",
        INPUT_PATH "findable-partials-100-xir.saved",
        INPUT_PATH "findable-partials-100-xmf.saved",
        INPUT_PATH "findable-partials-100-xwp.saved",
        INPUT_PATH "findable-partials-100-xxh.saved",
        INPUT_PATH "findable-partials-100-xzaap.saved",
        INPUT_PATH "findable-partials-100-xzabv.saved",
        INPUT_PATH "findable-partials-100-xvk.saved",
        INPUT_PATH "findable-partials-100-xcv.saved",
        INPUT_PATH "findable-partials-100-xns.saved",
        INPUT_PATH "findable-partials-100-xzaii.saved",
        INPUT_PATH "findable-partials-100-xyz.saved",
        INPUT_PATH "findable-partials-100-xvt.saved",
        INPUT_PATH "findable-partials-100-xdd.saved",
        INPUT_PATH "findable-partials-100-xzahm.saved",
        INPUT_PATH "findable-partials-100-xzack.saved",
        INPUT_PATH "findable-partials-100-xkp.saved",
        INPUT_PATH "findable-partials-100-xzahu.saved",
        INPUT_PATH "findable-partials-100-xxr.saved",
        INPUT_PATH "findable-partials-100-xzaai.saved",
        INPUT_PATH "findable-partials-100-xzadr.saved",
        INPUT_PATH "findable-partials-100-xll.saved",
        INPUT_PATH "findable-partials-100-xiq.saved",
        INPUT_PATH "findable-partials-100-xzaij.saved",
        INPUT_PATH "findable-partials-100-xob.saved",
        INPUT_PATH "findable-partials-100-xos.saved",
        INPUT_PATH "findable-partials-100-xzaef.saved",
        INPUT_PATH "findable-partials-100-xqg.saved",
        INPUT_PATH "findable-partials-100-xzaik.saved",
        INPUT_PATH "findable-partials-100-xdj.saved",
        INPUT_PATH "findable-partials-100-xbq.saved",
        INPUT_PATH "findable-partials-100-xjb.saved",
        INPUT_PATH "findable-partials-100-xpf.saved" } },
    { false,
      ORDER_PATH_FMT "-findable-10000",
      6394630,
      { INPUT_PATH "findable-partials-10000-xad.saved",
        INPUT_PATH "findable-partials-10000-xab.saved",
        INPUT_PATH "findable-partials-10000-xaf.saved",
        INPUT_PATH "findable-partials-10000-xag.saved",
        INPUT_PATH "findable-partials-10000-xah.saved",
        INPUT_PATH "findable-partials-10000-xaa.saved",
        INPUT_PATH "findable-partials-10000-xac.saved",
        INPUT_PATH "findable-partials-10000-xai.saved",
        INPUT_PATH "findable-partials-10000-xae.saved" } },
    { false,
      ORDER_PATH_FMT "-findable-25000",
      6394630,
      { INPUT_PATH "findable-partials-25000-xad.saved",
        INPUT_PATH "findable-partials-25000-xab.saved",
        INPUT_PATH "findable-partials-25000-xaa.saved",
        INPUT_PATH "findable-partials-25000-xac.saved" } },
    { false,
      ORDER_PATH_FMT "-findable-1000",
      6394630,
      { INPUT_PATH "findable-partials-1000-xct.saved",
        INPUT_PATH "findable-partials-1000-xam.saved",
        INPUT_PATH "findable-partials-1000-xcm.saved",
        INPUT_PATH "findable-partials-1000-xce.saved",
        INPUT_PATH "findable-partials-1000-xbj.saved",
        INPUT_PATH "findable-partials-1000-xcn.saved",
        INPUT_PATH "findable-partials-1000-xak.saved",
        INPUT_PATH "findable-partials-1000-xad.saved",
        INPUT_PATH "findable-partials-1000-xav.saved",
        INPUT_PATH "findable-partials-1000-xbv.saved",
        INPUT_PATH "findable-partials-1000-xde.saved",
        INPUT_PATH "findable-partials-1000-xaj.saved",
        INPUT_PATH "findable-partials-1000-xbo.saved",
        INPUT_PATH "findable-partials-1000-xat.saved",
        INPUT_PATH "findable-partials-1000-xcc.saved",
        INPUT_PATH "findable-partials-1000-xbm.saved",
        INPUT_PATH "findable-partials-1000-xch.saved",
        INPUT_PATH "findable-partials-1000-xbt.saved",
        INPUT_PATH "findable-partials-1000-xan.saved",
        INPUT_PATH "findable-partials-1000-xbx.saved",
        INPUT_PATH "findable-partials-1000-xbz.saved",
        INPUT_PATH "findable-partials-1000-xbf.saved",
        INPUT_PATH "findable-partials-1000-xbw.saved",
        INPUT_PATH "findable-partials-1000-xau.saved",
        INPUT_PATH "findable-partials-1000-xcu.saved",
        INPUT_PATH "findable-partials-1000-xab.saved",
        INPUT_PATH "findable-partials-1000-xao.saved",
        INPUT_PATH "findable-partials-1000-xcz.saved",
        INPUT_PATH "findable-partials-1000-xbb.saved",
        INPUT_PATH "findable-partials-1000-xdh.saved",
        INPUT_PATH "findable-partials-1000-xar.saved",
        INPUT_PATH "findable-partials-1000-xaf.saved",
        INPUT_PATH "findable-partials-1000-xba.saved",
        INPUT_PATH "findable-partials-1000-xcq.saved",
        INPUT_PATH "findable-partials-1000-xaw.saved",
        INPUT_PATH "findable-partials-1000-xbu.saved",
        INPUT_PATH "findable-partials-1000-xbl.saved",
        INPUT_PATH "findable-partials-1000-xag.saved",
        INPUT_PATH "findable-partials-1000-xbi.saved",
        INPUT_PATH "findable-partials-1000-xbd.saved",
        INPUT_PATH "findable-partials-1000-xcf.saved",
        INPUT_PATH "findable-partials-1000-xby.saved",
        INPUT_PATH "findable-partials-1000-xaz.saved",
        INPUT_PATH "findable-partials-1000-xax.saved",
        INPUT_PATH "findable-partials-1000-xah.saved",
        INPUT_PATH "findable-partials-1000-xcd.saved",
        INPUT_PATH "findable-partials-1000-xbp.saved",
        INPUT_PATH "findable-partials-1000-xbn.saved",
        INPUT_PATH "findable-partials-1000-xcy.saved",
        INPUT_PATH "findable-partials-1000-xaa.saved",
        INPUT_PATH "findable-partials-1000-xaq.saved",
        INPUT_PATH "findable-partials-1000-xbh.saved",
        INPUT_PATH "findable-partials-1000-xap.saved",
        INPUT_PATH "findable-partials-1000-xbc.saved",
        INPUT_PATH "findable-partials-1000-xcr.saved",
        INPUT_PATH "findable-partials-1000-xca.saved",
        INPUT_PATH "findable-partials-1000-xac.saved",
        INPUT_PATH "findable-partials-1000-xdc.saved",
        INPUT_PATH "findable-partials-1000-xbs.saved",
        INPUT_PATH "findable-partials-1000-xdk.saved",
        INPUT_PATH "findable-partials-1000-xco.saved",
        INPUT_PATH "findable-partials-1000-xai.saved",
        INPUT_PATH "findable-partials-1000-xci.saved",
        INPUT_PATH "findable-partials-1000-xdf.saved",
        INPUT_PATH "findable-partials-1000-xdi.saved",
        INPUT_PATH "findable-partials-1000-xbk.saved",
        INPUT_PATH "findable-partials-1000-xck.saved",
        INPUT_PATH "findable-partials-1000-xda.saved",
        INPUT_PATH "findable-partials-1000-xcs.saved",
        INPUT_PATH "findable-partials-1000-xcb.saved",
        INPUT_PATH "findable-partials-1000-xbe.saved",
        INPUT_PATH "findable-partials-1000-xay.saved",
        INPUT_PATH "findable-partials-1000-xcl.saved",
        INPUT_PATH "findable-partials-1000-xbg.saved",
        INPUT_PATH "findable-partials-1000-xbr.saved",
        INPUT_PATH "findable-partials-1000-xcw.saved",
        INPUT_PATH "findable-partials-1000-xcj.saved",
        INPUT_PATH "findable-partials-1000-xdb.saved",
        INPUT_PATH "findable-partials-1000-xas.saved",
        INPUT_PATH "findable-partials-1000-xae.saved",
        INPUT_PATH "findable-partials-1000-xdg.saved",
        INPUT_PATH "findable-partials-1000-xcp.saved",
        INPUT_PATH "findable-partials-1000-xcx.saved",
        INPUT_PATH "findable-partials-1000-xal.saved",
        INPUT_PATH "findable-partials-1000-xcg.saved",
        INPUT_PATH "findable-partials-1000-xcv.saved",
        INPUT_PATH "findable-partials-1000-xdd.saved",
        INPUT_PATH "findable-partials-1000-xdj.saved",
        INPUT_PATH "findable-partials-1000-xbq.saved" } },
    { false,
      ORDER_PATH_FMT "-findable-5000",
      6394630,
      { INPUT_PATH "findable-partials-5000-xam.saved",
        INPUT_PATH "findable-partials-5000-xak.saved",
        INPUT_PATH "findable-partials-5000-xad.saved",
        INPUT_PATH "findable-partials-5000-xaj.saved",
        INPUT_PATH "findable-partials-5000-xan.saved",
        INPUT_PATH "findable-partials-5000-xab.saved",
        INPUT_PATH "findable-partials-5000-xao.saved",
        INPUT_PATH "findable-partials-5000-xar.saved",
        INPUT_PATH "findable-partials-5000-xaf.saved",
        INPUT_PATH "findable-partials-5000-xag.saved",
        INPUT_PATH "findable-partials-5000-xah.saved",
        INPUT_PATH "findable-partials-5000-xaa.saved",
        INPUT_PATH "findable-partials-5000-xaq.saved",
        INPUT_PATH "findable-partials-5000-xap.saved",
        INPUT_PATH "findable-partials-5000-xac.saved",
        INPUT_PATH "findable-partials-5000-xai.saved",
        INPUT_PATH "findable-partials-5000-xae.saved",
        INPUT_PATH "findable-partials-5000-xal.saved" } },
    { false,
      ORDER_PATH_FMT "-findable-50000",
      6394630,
      { INPUT_PATH "findable-partials-50000-xab.saved",
        INPUT_PATH "findable-partials-50000-xaa.saved" } },
    { true,
      ORDER_PATH_FMT "-unknowns-2500",
      49070782257,
      { INPUT_PATH "unknowns-partials-2500-xam.saved",
        INPUT_PATH "unknowns-partials-2500-xbj.saved",
        INPUT_PATH "unknowns-partials-2500-xak.saved",
        INPUT_PATH "unknowns-partials-2500-xad.saved",
        INPUT_PATH "unknowns-partials-2500-xav.saved",
        INPUT_PATH "unknowns-partials-2500-xaj.saved",
        INPUT_PATH "unknowns-partials-2500-xat.saved",
        INPUT_PATH "unknowns-partials-2500-xan.saved",
        INPUT_PATH "unknowns-partials-2500-xbf.saved",
        INPUT_PATH "unknowns-partials-2500-xau.saved",
        INPUT_PATH "unknowns-partials-2500-xab.saved",
        INPUT_PATH "unknowns-partials-2500-xao.saved",
        INPUT_PATH "unknowns-partials-2500-xbb.saved",
        INPUT_PATH "unknowns-partials-2500-xar.saved",
        INPUT_PATH "unknowns-partials-2500-xaf.saved",
        INPUT_PATH "unknowns-partials-2500-xba.saved",
        INPUT_PATH "unknowns-partials-2500-xaw.saved",
        INPUT_PATH "unknowns-partials-2500-xag.saved",
        INPUT_PATH "unknowns-partials-2500-xbi.saved",
        INPUT_PATH "unknowns-partials-2500-xbd.saved",
        INPUT_PATH "unknowns-partials-2500-xaz.saved",
        INPUT_PATH "unknowns-partials-2500-xax.saved",
        INPUT_PATH "unknowns-partials-2500-xah.saved",
        INPUT_PATH "unknowns-partials-2500-xaa.saved",
        INPUT_PATH "unknowns-partials-2500-xaq.saved",
        INPUT_PATH "unknowns-partials-2500-xbh.saved",
        INPUT_PATH "unknowns-partials-2500-xap.saved",
        INPUT_PATH "unknowns-partials-2500-xbc.saved",
        INPUT_PATH "unknowns-partials-2500-xac.saved",
        INPUT_PATH "unknowns-partials-2500-xai.saved",
        INPUT_PATH "unknowns-partials-2500-xbe.saved",
        INPUT_PATH "unknowns-partials-2500-xay.saved",
        INPUT_PATH "unknowns-partials-2500-xbg.saved",
        INPUT_PATH "unknowns-partials-2500-xas.saved",
        INPUT_PATH "unknowns-partials-2500-xae.saved",
        INPUT_PATH "unknowns-partials-2500-xal.saved" } },
    { true,
      ORDER_PATH_FMT "-unknowns-100",
      49065563576,
      { INPUT_PATH "unknowns-partials-100-xql.saved",
        INPUT_PATH "unknowns-partials-100-xzaga.saved",
        INPUT_PATH "unknowns-partials-100-xmq.saved",
        INPUT_PATH "unknowns-partials-100-xpd.saved",
        INPUT_PATH "unknowns-partials-100-xzafd.saved",
        INPUT_PATH "unknowns-partials-100-xva.saved",
        INPUT_PATH "unknowns-partials-100-xzaid.saved",
        INPUT_PATH "unknowns-partials-100-xzaak.saved",
        INPUT_PATH "unknowns-partials-100-xzadz.saved",
        INPUT_PATH "unknowns-partials-100-xml.saved",
        INPUT_PATH "unknowns-partials-100-xct.saved",
        INPUT_PATH "unknowns-partials-100-xuq.saved",
        INPUT_PATH "unknowns-partials-100-xyr.saved",
        INPUT_PATH "unknowns-partials-100-xzadw.saved",
        INPUT_PATH "unknowns-partials-100-xor.saved",
        INPUT_PATH "unknowns-partials-100-xkn.saved",
        INPUT_PATH "unknowns-partials-100-xam.saved",
        INPUT_PATH "unknowns-partials-100-xzaca.saved",
        INPUT_PATH "unknowns-partials-100-xtq.saved",
        INPUT_PATH "unknowns-partials-100-xnn.saved",
        INPUT_PATH "unknowns-partials-100-xmd.saved",
        INPUT_PATH "unknowns-partials-100-xlz.saved",
        INPUT_PATH "unknowns-partials-100-xzaic.saved",
        INPUT_PATH "unknowns-partials-100-xzagj.saved",
        INPUT_PATH "unknowns-partials-100-xzafu.saved",
        INPUT_PATH "unknowns-partials-100-xnw.saved",
        INPUT_PATH "unknowns-partials-100-xks.saved",
        INPUT_PATH "unknowns-partials-100-xoh.saved",
        INPUT_PATH "unknowns-partials-100-xzagy.saved",
        INPUT_PATH "unknowns-partials-100-xxd.saved",
        INPUT_PATH "unknowns-partials-100-xve.saved",
        INPUT_PATH "unknowns-partials-100-xng.saved",
        INPUT_PATH "unknowns-partials-100-xep.saved",
        INPUT_PATH "unknowns-partials-100-xzaia.saved",
        INPUT_PATH "unknowns-partials-100-xcm.saved",
        INPUT_PATH "unknowns-partials-100-xwn.saved",
        INPUT_PATH "unknowns-partials-100-xlb.saved",
        INPUT_PATH "unknowns-partials-100-xce.saved",
        INPUT_PATH "unknowns-partials-100-xzaaz.saved",
        INPUT_PATH "unknowns-partials-100-xgx.saved",
        INPUT_PATH "unknowns-partials-100-xzaha.saved",
        INPUT_PATH "unknowns-partials-100-xzaib.saved",
        INPUT_PATH "unknowns-partials-100-xjo.saved",
        INPUT_PATH "unknowns-partials-100-xle.saved",
        INPUT_PATH "unknowns-partials-100-xjq.saved",
        INPUT_PATH "unknowns-partials-100-xta.saved",
        INPUT_PATH "unknowns-partials-100-xzafm.saved",
        INPUT_PATH "unknowns-partials-100-xws.saved",
        INPUT_PATH "unknowns-partials-100-xzaip.saved",
        INPUT_PATH "unknowns-partials-100-xis.saved",
        INPUT_PATH "unknowns-partials-100-xnv.saved",
        INPUT_PATH "unknowns-partials-100-xie.saved",
        INPUT_PATH "unknowns-partials-100-xbj.saved",
        INPUT_PATH "unknowns-partials-100-xzahs.saved",
        INPUT_PATH "unknowns-partials-100-xga.saved",
        INPUT_PATH "unknowns-partials-100-xgi.saved",
        INPUT_PATH "unknowns-partials-100-xzadh.saved",
        INPUT_PATH "unknowns-partials-100-xsz.saved",
        INPUT_PATH "unknowns-partials-100-xds.saved",
        INPUT_PATH "unknowns-partials-100-xrf.saved",
        INPUT_PATH "unknowns-partials-100-xqv.saved",
        INPUT_PATH "unknowns-partials-100-xrs.saved",
        INPUT_PATH "unknowns-partials-100-xzagv.saved",
        INPUT_PATH "unknowns-partials-100-xzain.saved",
        INPUT_PATH "unknowns-partials-100-xtn.saved",
        INPUT_PATH "unknowns-partials-100-xcn.saved",
        INPUT_PATH "unknowns-partials-100-xtt.saved",
        INPUT_PATH "unknowns-partials-100-xgl.saved",
        INPUT_PATH "unknowns-partials-100-xak.saved",
        INPUT_PATH "unknowns-partials-100-xqp.saved",
        INPUT_PATH "unknowns-partials-100-xad.saved",
        INPUT_PATH "unknowns-partials-100-xoo.saved",
        INPUT_PATH "unknowns-partials-100-xzagt.saved",
        INPUT_PATH "unknowns-partials-100-xzaec.saved",
        INPUT_PATH "unknowns-partials-100-xum.saved",
        INPUT_PATH "unknowns-partials-100-xzabe.saved",
        INPUT_PATH "unknowns-partials-100-xko.saved",
        INPUT_PATH "unknowns-partials-100-xzaal.saved",
        INPUT_PATH "unknowns-partials-100-xzaad.saved",
        INPUT_PATH "unknowns-partials-100-xzabz.saved",
        INPUT_PATH "unknowns-partials-100-xav.saved",
        INPUT_PATH "unknowns-partials-100-xjm.saved",
        INPUT_PATH "unknowns-partials-100-xqq.saved",
        INPUT_PATH "unknowns-partials-100-xqn.saved",
        INPUT_PATH "unknowns-partials-100-xqc.saved",
        INPUT_PATH "unknowns-partials-100-xzabs.saved",
        INPUT_PATH "unknowns-partials-100-xxa.saved",
        INPUT_PATH "unknowns-partials-100-xzagc.saved",
        INPUT_PATH "unknowns-partials-100-xzaco.saved",
        INPUT_PATH "unknowns-partials-100-xzafe.saved",
        INPUT_PATH "unknowns-partials-100-xbv.saved",
        INPUT_PATH "unknowns-partials-100-xzacr.saved",
        INPUT_PATH "unknowns-partials-100-xde.saved",
        INPUT_PATH "unknowns-partials-100-xsw.saved",
        INPUT_PATH "unknowns-partials-100-xzaen.saved",
        INPUT_PATH "unknowns-partials-100-xee.saved",
        INPUT_PATH "unknowns-partials-100-xkj.saved",
        INPUT_PATH "unknowns-partials-100-xni.saved",
        INPUT_PATH "unknowns-partials-100-xzafl.saved",
        INPUT_PATH "unknowns-partials-100-xif.saved",
        INPUT_PATH "unknowns-partials-100-xrw.saved",
        INPUT_PATH "unknowns-partials-100-xzaiy.saved",
        INPUT_PATH "unknowns-partials-100-xzabc.saved",
        INPUT_PATH "unknowns-partials-100-xhk.saved",
        INPUT_PATH "unknowns-partials-100-xzagi.saved",
        INPUT_PATH "unknowns-partials-100-xzacp.saved",
        INPUT_PATH "unknowns-partials-100-xoe.saved",
        INPUT_PATH "unknowns-partials-100-xms.saved",
        INPUT_PATH "unknowns-partials-100-xik.saved",
        INPUT_PATH "unknowns-partials-100-xzagh.saved",
        INPUT_PATH "unknowns-partials-100-xjj.saved",
        INPUT_PATH "unknowns-partials-100-xmp.saved",
        INPUT_PATH "unknowns-partials-100-xld.saved",
        INPUT_PATH "unknowns-partials-100-xwy.saved",
        INPUT_PATH "unknowns-partials-100-xzafs.saved",
        INPUT_PATH "unknowns-partials-100-xef.saved",
        INPUT_PATH "unknowns-partials-100-xzaeb.saved",
        INPUT_PATH "unknowns-partials-100-xdl.saved",
        INPUT_PATH "unknowns-partials-100-xrr.saved",
        INPUT_PATH "unknowns-partials-100-xqj.saved",
        INPUT_PATH "unknowns-partials-100-xps.saved",
        INPUT_PATH "unknowns-partials-100-xvj.saved",
        INPUT_PATH "unknowns-partials-100-xyg.saved",
        INPUT_PATH "unknowns-partials-100-xkt.saved",
        INPUT_PATH "unknowns-partials-100-xxg.saved",
        INPUT_PATH "unknowns-partials-100-xxj.saved",
        INPUT_PATH "unknowns-partials-100-xju.saved",
        INPUT_PATH "unknowns-partials-100-xjs.saved",
        INPUT_PATH "unknowns-partials-100-xsb.saved",
        INPUT_PATH "unknowns-partials-100-xhw.saved",
        INPUT_PATH "unknowns-partials-100-xzadv.saved",
        INPUT_PATH "unknowns-partials-100-xhe.saved",
        INPUT_PATH "unknowns-partials-100-xlm.saved",
        INPUT_PATH "unknowns-partials-100-xzadx.saved",
        INPUT_PATH "unknowns-partials-100-xii.saved",
        INPUT_PATH "unknowns-partials-100-xwr.saved",
        INPUT_PATH "unknowns-partials-100-xnk.saved",
        INPUT_PATH "unknowns-partials-100-xxk.saved",
        INPUT_PATH "unknowns-partials-100-xzaig.saved",
        INPUT_PATH "unknowns-partials-100-xlo.saved",
        INPUT_PATH "unknowns-partials-100-xzael.saved",
        INPUT_PATH "unknowns-partials-100-xaj.saved",
        INPUT_PATH "unknowns-partials-100-xen.saved",
        INPUT_PATH "unknowns-partials-100-xmz.saved",
        INPUT_PATH "unknowns-partials-100-xsg.saved",
        INPUT_PATH "unknowns-partials-100-xbo.saved",
        INPUT_PATH "unknowns-partials-100-xyj.saved",
        INPUT_PATH "unknowns-partials-100-xzacf.saved",
        INPUT_PATH "unknowns-partials-100-xrc.saved",
        INPUT_PATH "unknowns-partials-100-xhd.saved",
        INPUT_PATH "unknowns-partials-100-xrp.saved",
        INPUT_PATH "unknowns-partials-100-xat.saved",
        INPUT_PATH "unknowns-partials-100-xka.saved",
        INPUT_PATH "unknowns-partials-100-xma.saved",
        INPUT_PATH "unknowns-partials-100-xer.saved",
        INPUT_PATH "unknowns-partials-100-xss.saved",
        INPUT_PATH "unknowns-partials-100-xwg.saved",
        INPUT_PATH "unknowns-partials-100-xhc.saved",
        INPUT_PATH "unknowns-partials-100-xzabw.saved",
        INPUT_PATH "unknowns-partials-100-xfl.saved",
        INPUT_PATH "unknowns-partials-100-xok.saved",
        INPUT_PATH "unknowns-partials-100-xzagk.saved",
        INPUT_PATH "unknowns-partials-100-xtb.saved",
        INPUT_PATH "unknowns-partials-100-xoa.saved",
        INPUT_PATH "unknowns-partials-100-xwd.saved",
        INPUT_PATH "unknowns-partials-100-xij.saved",
        INPUT_PATH "unknowns-partials-100-xpl.saved",
        INPUT_PATH "unknowns-partials-100-xdu.saved",
        INPUT_PATH "unknowns-partials-100-xsa.saved",
        INPUT_PATH "unknowns-partials-100-xzacg.saved",
        INPUT_PATH "unknowns-partials-100-xel.saved",
        INPUT_PATH "unknowns-partials-100-xzafn.saved",
        INPUT_PATH "unknowns-partials-100-xoj.saved",
        INPUT_PATH "unknowns-partials-100-xzagu.saved",
        INPUT_PATH "unknowns-partials-100-xtl.saved",
        INPUT_PATH "unknowns-partials-100-xkz.saved",
        INPUT_PATH "unknowns-partials-100-xzais.saved",
        INPUT_PATH "unknowns-partials-100-xmo.saved",
        INPUT_PATH "unknowns-partials-100-xfz.saved",
        INPUT_PATH "unknowns-partials-100-xzaag.saved",
        INPUT_PATH "unknowns-partials-100-xzagd.saved",
        INPUT_PATH "unknowns-partials-100-xhp.saved",
        INPUT_PATH "unknowns-partials-100-xzach.saved",
        INPUT_PATH "unknowns-partials-100-xgv.saved",
        INPUT_PATH "unknowns-partials-100-xyw.saved",
        INPUT_PATH "unknowns-partials-100-xfe.saved",
        INPUT_PATH "unknowns-partials-100-xjv.saved",
        INPUT_PATH "unknowns-partials-100-xgp.saved",
        INPUT_PATH "unknowns-partials-100-xpg.saved",
        INPUT_PATH "unknowns-partials-100-xlk.saved",
        INPUT_PATH "unknowns-partials-100-xxt.saved",
        INPUT_PATH "unknowns-partials-100-xuy.saved",
        INPUT_PATH "unknowns-partials-100-xzade.saved",
        INPUT_PATH "unknowns-partials-100-xzagm.saved",
        INPUT_PATH "unknowns-partials-100-xpu.saved",
        INPUT_PATH "unknowns-partials-100-xrk.saved",
        INPUT_PATH "unknowns-partials-100-xzagw.saved",
        INPUT_PATH "unknowns-partials-100-xkw.saved",
        INPUT_PATH "unknowns-partials-100-xzahk.saved",
        INPUT_PATH "unknowns-partials-100-xwz.saved",
        INPUT_PATH "unknowns-partials-100-xww.saved",
        INPUT_PATH "unknowns-partials-100-xzacj.saved",
        INPUT_PATH "unknowns-partials-100-xuj.saved",
        INPUT_PATH "unknowns-partials-100-xzaej.saved",
        INPUT_PATH "unknowns-partials-100-xcc.saved",
        INPUT_PATH "unknowns-partials-100-xst.saved",
        INPUT_PATH "unknowns-partials-100-xxe.saved",
        INPUT_PATH "unknowns-partials-100-xun.saved",
        INPUT_PATH "unknowns-partials-100-xzafq.saved",
        INPUT_PATH "unknowns-partials-100-xzaht.saved",
        INPUT_PATH "unknowns-partials-100-xqs.saved",
        INPUT_PATH "unknowns-partials-100-xno.saved",
        INPUT_PATH "unknowns-partials-100-xkm.saved",
        INPUT_PATH "unknowns-partials-100-xzacw.saved",
        INPUT_PATH "unknowns-partials-100-xzahv.saved",
        INPUT_PATH "unknowns-partials-100-xzadl.saved",
        INPUT_PATH "unknowns-partials-100-xxf.saved",
        INPUT_PATH "unknowns-partials-100-xhi.saved",
        INPUT_PATH "unknowns-partials-100-xqo.saved",
        INPUT_PATH "unknowns-partials-100-xls.saved",
        INPUT_PATH "unknowns-partials-100-xzagb.saved",
        INPUT_PATH "unknowns-partials-100-xzafc.saved",
        INPUT_PATH "unknowns-partials-100-xiz.saved",
        INPUT_PATH "unknowns-partials-100-xub.saved",
        INPUT_PATH "unknowns-partials-100-xro.saved",
        INPUT_PATH "unknowns-partials-100-xip.saved",
        INPUT_PATH "unknowns-partials-100-xtx.saved",
        INPUT_PATH "unknowns-partials-100-xzacd.saved",
        INPUT_PATH "unknowns-partials-100-xzahw.saved",
        INPUT_PATH "unknowns-partials-100-xbm.saved",
        INPUT_PATH "unknowns-partials-100-xzacq.saved",
        INPUT_PATH "unknowns-partials-100-xpc.saved",
        INPUT_PATH "unknowns-partials-100-xtu.saved",
        INPUT_PATH "unknowns-partials-100-xzafo.saved",
        INPUT_PATH "unknowns-partials-100-xzafy.saved",
        INPUT_PATH "unknowns-partials-100-xxp.saved",
        INPUT_PATH "unknowns-partials-100-xeb.saved",
        INPUT_PATH "unknowns-partials-100-xzabx.saved",
        INPUT_PATH "unknowns-partials-100-xzadn.saved",
        INPUT_PATH "unknowns-partials-100-xxi.saved",
        INPUT_PATH "unknowns-partials-100-xvo.saved",
        INPUT_PATH "unknowns-partials-100-xyn.saved",
        INPUT_PATH "unknowns-partials-100-xfr.saved",
        INPUT_PATH "unknowns-partials-100-xvy.saved",
        INPUT_PATH "unknowns-partials-100-xod.saved",
        INPUT_PATH "unknowns-partials-100-xiu.saved",
        INPUT_PATH "unknowns-partials-100-xch.saved",
        INPUT_PATH "unknowns-partials-100-xzabk.saved",
        INPUT_PATH "unknowns-partials-100-xdr.saved",
        INPUT_PATH "unknowns-partials-100-xzact.saved",
        INPUT_PATH "unknowns-partials-100-xft.saved",
        INPUT_PATH "unknowns-partials-100-xvd.saved",
        INPUT_PATH "unknowns-partials-100-xwh.saved",
        INPUT_PATH "unknowns-partials-100-xzail.saved",
        INPUT_PATH "unknowns-partials-100-xxy.saved",
        INPUT_PATH "unknowns-partials-100-xyk.saved",
        INPUT_PATH "unknowns-partials-100-xmn.saved",
        INPUT_PATH "unknowns-partials-100-xfo.saved",
        INPUT_PATH "unknowns-partials-100-xqr.saved",
        INPUT_PATH "unknowns-partials-100-xzadi.saved",
        INPUT_PATH "unknowns-partials-100-xoy.saved",
        INPUT_PATH "unknowns-partials-100-xzaae.saved",
        INPUT_PATH "unknowns-partials-100-xbt.saved",
        INPUT_PATH "unknowns-partials-100-xzads.saved",
        INPUT_PATH "unknowns-partials-100-xzaiz.saved",
        INPUT_PATH "unknowns-partials-100-xan.saved",
        INPUT_PATH "unknowns-partials-100-xso.saved",
        INPUT_PATH "unknowns-partials-100-xei.saved",
        INPUT_PATH "unknowns-partials-100-xyd.saved",
        INPUT_PATH "unknowns-partials-100-xgz.saved",
        INPUT_PATH "unknowns-partials-100-xfq.saved",
        INPUT_PATH "unknowns-partials-100-xzaih.saved",
        INPUT_PATH "unknowns-partials-100-xwc.saved",
        INPUT_PATH "unknowns-partials-100-xes.saved",
        INPUT_PATH "unknowns-partials-100-xzacl.saved",
        INPUT_PATH "unknowns-partials-100-xbx.saved",
        INPUT_PATH "unknowns-partials-100-xjw.saved",
        INPUT_PATH "unknowns-partials-100-xzacc.saved",
        INPUT_PATH "unknowns-partials-100-xex.saved",
        INPUT_PATH "unknowns-partials-100-xon.saved",
        INPUT_PATH "unknowns-partials-100-xvw.saved",
        INPUT_PATH "unknowns-partials-100-xwe.saved",
        INPUT_PATH "unknowns-partials-100-xfd.saved",
        INPUT_PATH "unknowns-partials-100-xvi.saved",
        INPUT_PATH "unknowns-partials-100-xkk.saved",
        INPUT_PATH "unknowns-partials-100-xzaaa.saved",
        INPUT_PATH "unknowns-partials-100-xzagl.saved",
        INPUT_PATH "unknowns-partials-100-xfc.saved",
        INPUT_PATH "unknowns-partials-100-xzahl.saved",
        INPUT_PATH "unknowns-partials-100-xev.saved",
        INPUT_PATH "unknowns-partials-100-xdy.saved",
        INPUT_PATH "unknowns-partials-100-xbz.saved",
        INPUT_PATH "unknowns-partials-100-xsu.saved",
        INPUT_PATH "unknowns-partials-100-xzaah.saved",
        INPUT_PATH "unknowns-partials-100-xgh.saved",
        INPUT_PATH "unknowns-partials-100-xmt.saved",
        INPUT_PATH "unknowns-partials-100-xty.saved",
        INPUT_PATH "unknowns-partials-100-xzahp.saved",
        INPUT_PATH "unknowns-partials-100-xia.saved",
        INPUT_PATH "unknowns-partials-100-xsc.saved",
        INPUT_PATH "unknowns-partials-100-xnd.saved",
        INPUT_PATH "unknowns-partials-100-xeu.saved",
        INPUT_PATH "unknowns-partials-100-xzaeq.saved",
        INPUT_PATH "unknowns-partials-100-xpa.saved",
        INPUT_PATH "unknowns-partials-100-xzaer.saved",
        INPUT_PATH "unknowns-partials-100-xuf.saved",
        INPUT_PATH "unknowns-partials-100-xsq.saved",
        INPUT_PATH "unknowns-partials-100-xuo.saved",
        INPUT_PATH "unknowns-partials-100-xbf.saved",
        INPUT_PATH "unknowns-partials-100-xdp.saved",
        INPUT_PATH "unknowns-partials-100-xwa.saved",
        INPUT_PATH "unknowns-partials-100-xzaim.saved",
        INPUT_PATH "unknowns-partials-100-xtp.saved",
        INPUT_PATH "unknowns-partials-100-xri.saved",
        INPUT_PATH "unknowns-partials-100-xzaaj.saved",
        INPUT_PATH "unknowns-partials-100-xzahe.saved",
        INPUT_PATH "unknowns-partials-100-xhh.saved",
        INPUT_PATH "unknowns-partials-100-xra.saved",
        INPUT_PATH "unknowns-partials-100-xzagr.saved",
        INPUT_PATH "unknowns-partials-100-xud.saved",
        INPUT_PATH "unknowns-partials-100-xry.saved",
        INPUT_PATH "unknowns-partials-100-xgg.saved",
        INPUT_PATH "unknowns-partials-100-xzaff.saved",
        INPUT_PATH "unknowns-partials-100-xsm.saved",
        INPUT_PATH "unknowns-partials-100-xkd.saved",
        INPUT_PATH "unknowns-partials-100-xgm.saved",
        INPUT_PATH "unknowns-partials-100-xiv.saved",
        INPUT_PATH "unknowns-partials-100-xfa.saved",
        INPUT_PATH "unknowns-partials-100-xlq.saved",
        INPUT_PATH "unknowns-partials-100-xlc.saved",
        INPUT_PATH "unknowns-partials-100-xvc.saved",
        INPUT_PATH "unknowns-partials-100-xhq.saved",
        INPUT_PATH "unknowns-partials-100-xlu.saved",
        INPUT_PATH "unknowns-partials-100-xbw.saved",
        INPUT_PATH "unknowns-partials-100-xya.saved",
        INPUT_PATH "unknowns-partials-100-xzafz.saved",
        INPUT_PATH "unknowns-partials-100-xzago.saved",
        INPUT_PATH "unknowns-partials-100-xhn.saved",
        INPUT_PATH "unknowns-partials-100-xwt.saved",
        INPUT_PATH "unknowns-partials-100-xkv.saved",
        INPUT_PATH "unknowns-partials-100-xkx.saved",
        INPUT_PATH "unknowns-partials-100-xfu.saved",
        INPUT_PATH "unknowns-partials-100-xau.saved",
        INPUT_PATH "unknowns-partials-100-xzaaw.saved",
        INPUT_PATH "unknowns-partials-100-xjn.saved",
        INPUT_PATH "unknowns-partials-100-xzaat.saved",
        INPUT_PATH "unknowns-partials-100-xzabm.saved",
        INPUT_PATH "unknowns-partials-100-xcu.saved",
        INPUT_PATH "unknowns-partials-100-xmw.saved",
        INPUT_PATH "unknowns-partials-100-xjd.saved",
        INPUT_PATH "unknowns-partials-100-xnm.saved",
        INPUT_PATH "unknowns-partials-100-xzaio.saved",
        INPUT_PATH "unknowns-partials-100-xab.saved",
        INPUT_PATH "unknowns-partials-100-xzafr.saved",
        INPUT_PATH "unknowns-partials-100-xoi.saved",
        INPUT_PATH "unknowns-partials-100-xtz.saved",
        INPUT_PATH "unknowns-partials-100-xur.saved",
        INPUT_PATH "unknowns-partials-100-xeg.saved",
        INPUT_PATH "unknowns-partials-100-xnp.saved",
        INPUT_PATH "unknowns-partials-100-xdw.saved",
        INPUT_PATH "unknowns-partials-100-xom.saved",
        INPUT_PATH "unknowns-partials-100-xzadj.saved",
        INPUT_PATH "unknowns-partials-100-xzaie.saved",
        INPUT_PATH "unknowns-partials-100-xpz.saved",
        INPUT_PATH "unknowns-partials-100-xyc.saved",
        INPUT_PATH "unknowns-partials-100-xao.saved",
        INPUT_PATH "unknowns-partials-100-xcz.saved",
        INPUT_PATH "unknowns-partials-100-xtf.saved",
        INPUT_PATH "unknowns-partials-100-xlv.saved",
        INPUT_PATH "unknowns-partials-100-xog.saved",
        INPUT_PATH "unknowns-partials-100-xys.saved",
        INPUT_PATH "unknowns-partials-100-xmv.saved",
        INPUT_PATH "unknowns-partials-100-xsv.saved",
        INPUT_PATH "unknowns-partials-100-xbb.saved",
        INPUT_PATH "unknowns-partials-100-xjf.saved",
        INPUT_PATH "unknowns-partials-100-xig.saved",
        INPUT_PATH "unknowns-partials-100-xnl.saved",
        INPUT_PATH "unknowns-partials-100-xzaab.saved",
        INPUT_PATH "unknowns-partials-100-xzaix.saved",
        INPUT_PATH "unknowns-partials-100-xnj.saved",
        INPUT_PATH "unknowns-partials-100-xmb.saved",
        INPUT_PATH "unknowns-partials-100-xvp.saved",
        INPUT_PATH "unknowns-partials-100-xdh.saved",
        INPUT_PATH "unknowns-partials-100-xar.saved",
        INPUT_PATH "unknowns-partials-100-xru.saved",
        INPUT_PATH "unknowns-partials-100-xeo.saved",
        INPUT_PATH "unknowns-partials-100-xqu.saved",
        INPUT_PATH "unknowns-partials-100-xzaao.saved",
        INPUT_PATH "unknowns-partials-100-xtk.saved",
        INPUT_PATH "unknowns-partials-100-xpr.saved",
        INPUT_PATH "unknowns-partials-100-xli.saved",
        INPUT_PATH "unknowns-partials-100-xuc.saved",
        INPUT_PATH "unknowns-partials-100-xwv.saved",
        INPUT_PATH "unknowns-partials-100-xaf.saved",
        INPUT_PATH "unknowns-partials-100-xgw.saved",
        INPUT_PATH "unknowns-partials-100-xzadm.saved",
        INPUT_PATH "unknowns-partials-100-xfp.saved",
        INPUT_PATH "unknowns-partials-100-xnt.saved",
        INPUT_PATH "unknowns-partials-100-xzaez.saved",
        INPUT_PATH "unknowns-partials-100-xpp.saved",
        INPUT_PATH "unknowns-partials-100-xzabq.saved",
        INPUT_PATH "unknowns-partials-100-xlt.saved",
        INPUT_PATH "unknowns-partials-100-xqt.saved",
        INPUT_PATH "unknowns-partials-100-xzaba.saved",
        INPUT_PATH "unknowns-partials-100-xhm.saved",
        INPUT_PATH "unknowns-partials-100-xrm.saved",
        INPUT_PATH "unknowns-partials-100-xmg.saved",
        INPUT_PATH "unknowns-partials-100-xnx.saved",
        INPUT_PATH "unknowns-partials-100-xje.saved",
        INPUT_PATH "unknowns-partials-100-xba.saved",
        INPUT_PATH "unknowns-partials-100-xqk.saved",
        INPUT_PATH "unknowns-partials-100-xfh.saved",
        INPUT_PATH "unknowns-partials-100-xzaet.saved",
        INPUT_PATH "unknowns-partials-100-xvx.saved",
        INPUT_PATH "unknowns-partials-100-xvh.saved",
        INPUT_PATH "unknowns-partials-100-xyy.saved",
        INPUT_PATH "unknowns-partials-100-xpq.saved",
        INPUT_PATH "unknowns-partials-100-xja.saved",
        INPUT_PATH "unknowns-partials-100-xsx.saved",
        INPUT_PATH "unknowns-partials-100-xgj.saved",
        INPUT_PATH "unknowns-partials-100-xox.saved",
        INPUT_PATH "unknowns-partials-100-xtd.saved",
        INPUT_PATH "unknowns-partials-100-xvs.saved",
        INPUT_PATH "unknowns-partials-100-xzadf.saved",
        INPUT_PATH "unknowns-partials-100-xmi.saved",
        INPUT_PATH "unknowns-partials-100-xcq.saved",
        INPUT_PATH "unknowns-partials-100-xaw.saved",
        INPUT_PATH "unknowns-partials-100-xsf.saved",
        INPUT_PATH "unknowns-partials-100-xzaem.saved",
        INPUT_PATH "unknowns-partials-100-xlg.saved",
        INPUT_PATH "unknowns-partials-100-xzaes.saved",
        INPUT_PATH "unknowns-partials-100-xgt.saved",
        INPUT_PATH "unknowns-partials-100-xzadg.saved",
        INPUT_PATH "unknowns-partials-100-xqz.saved",
        INPUT_PATH "unknowns-partials-100-xhv.saved",
        INPUT_PATH "unknowns-partials-100-xej.saved",
        INPUT_PATH "unknowns-partials-100-xio.saved",
        INPUT_PATH "unknowns-partials-100-xzafk.saved",
        INPUT_PATH "unknowns-partials-100-xzaan.saved",
        INPUT_PATH "unknowns-partials-100-xuv.saved",
        INPUT_PATH "unknowns-partials-100-xzaiv.saved",
        INPUT_PATH "unknowns-partials-100-xdo.saved",
        INPUT_PATH "unknowns-partials-100-xsp.saved",
        INPUT_PATH "unknowns-partials-100-xff.saved",
        INPUT_PATH "unknowns-partials-100-xzafg.saved",
        INPUT_PATH "unknowns-partials-100-xdt.saved",
        INPUT_PATH "unknowns-partials-100-xzabp.saved",
        INPUT_PATH "unknowns-partials-100-xuu.saved",
        INPUT_PATH "unknowns-partials-100-xzafa.saved",
        INPUT_PATH "unknowns-partials-100-xbu.saved",
        INPUT_PATH "unknowns-partials-100-xji.saved",
        INPUT_PATH "unknowns-partials-100-xwl.saved",
        INPUT_PATH "unknowns-partials-100-xbl.saved",
        INPUT_PATH "unknowns-partials-100-xzahf.saved",
        INPUT_PATH "unknowns-partials-100-xsj.saved",
        INPUT_PATH "unknowns-partials-100-xvq.saved",
        INPUT_PATH "unknowns-partials-100-xyo.saved",
        INPUT_PATH "unknowns-partials-100-xib.saved",
        INPUT_PATH "unknowns-partials-100-xag.saved",
        INPUT_PATH "unknowns-partials-100-xgu.saved",
        INPUT_PATH "unknowns-partials-100-xbi.saved",
        INPUT_PATH "unknowns-partials-100-xrq.saved",
        INPUT_PATH "unknowns-partials-100-xyq.saved",
        INPUT_PATH "unknowns-partials-100-xbd.saved",
        INPUT_PATH "unknowns-partials-100-xln.saved",
        INPUT_PATH "unknowns-partials-100-xmm.saved",
        INPUT_PATH "unknowns-partials-100-xna.saved",
        INPUT_PATH "unknowns-partials-100-xyb.saved",
        INPUT_PATH "unknowns-partials-100-xkb.saved",
        INPUT_PATH "unknowns-partials-100-xou.saved",
        INPUT_PATH "unknowns-partials-100-xew.saved",
        INPUT_PATH "unknowns-partials-100-xiy.saved",
        INPUT_PATH "unknowns-partials-100-xxs.saved",
        INPUT_PATH "unknowns-partials-100-xcf.saved",
        INPUT_PATH "unknowns-partials-100-xzaee.saved",
        INPUT_PATH "unknowns-partials-100-xby.saved",
        INPUT_PATH "unknowns-partials-100-xih.saved",
        INPUT_PATH "unknowns-partials-100-xnh.saved",
        INPUT_PATH "unknowns-partials-100-xzabn.saved",
        INPUT_PATH "unknowns-partials-100-xue.saved",
        INPUT_PATH "unknowns-partials-100-xuz.saved",
        INPUT_PATH "unknowns-partials-100-xzaif.saved",
        INPUT_PATH "unknowns-partials-100-xvv.saved",
        INPUT_PATH "unknowns-partials-100-xol.saved",
        INPUT_PATH "unknowns-partials-100-xzahd.saved",
        INPUT_PATH "unknowns-partials-100-xul.saved",
        INPUT_PATH "unknowns-partials-100-xgo.saved",
        INPUT_PATH "unknowns-partials-100-xmr.saved",
        INPUT_PATH "unknowns-partials-100-xpw.saved",
        INPUT_PATH "unknowns-partials-100-xyt.saved",
        INPUT_PATH "unknowns-partials-100-xaz.saved",
        INPUT_PATH "unknowns-partials-100-xil.saved",
        INPUT_PATH "unknowns-partials-100-xzaaq.saved",
        INPUT_PATH "unknowns-partials-100-xla.saved",
        INPUT_PATH "unknowns-partials-100-xzadk.saved",
        INPUT_PATH "unknowns-partials-100-xnu.saved",
        INPUT_PATH "unknowns-partials-100-xvn.saved",
        INPUT_PATH "unknowns-partials-100-xuk.saved",
        INPUT_PATH "unknowns-partials-100-xzaaf.saved",
        INPUT_PATH "unknowns-partials-100-xne.saved",
        INPUT_PATH "unknowns-partials-100-xzaei.saved",
        INPUT_PATH "unknowns-partials-100-xkc.saved",
        INPUT_PATH "unknowns-partials-100-xua.saved",
        INPUT_PATH "unknowns-partials-100-xly.saved",
        INPUT_PATH "unknowns-partials-100-xrx.saved",
        INPUT_PATH "unknowns-partials-100-xzafp.saved",
        INPUT_PATH "unknowns-partials-100-xsn.saved",
        INPUT_PATH "unknowns-partials-100-xax.saved",
        INPUT_PATH "unknowns-partials-100-xzair.saved",
        INPUT_PATH "unknowns-partials-100-xyx.saved",
        INPUT_PATH "unknowns-partials-100-xpo.saved",
        INPUT_PATH "unknowns-partials-100-xqm.saved",
        INPUT_PATH "unknowns-partials-100-xzafb.saved",
        INPUT_PATH "unknowns-partials-100-xzaav.saved",
        INPUT_PATH "unknowns-partials-100-xoq.saved",
        INPUT_PATH "unknowns-partials-100-xin.saved",
        INPUT_PATH "unknowns-partials-100-xjg.saved",
        INPUT_PATH "unknowns-partials-100-xqw.saved",
        INPUT_PATH "unknowns-partials-100-xvm.saved",
        INPUT_PATH "unknowns-partials-100-xzahr.saved",
        INPUT_PATH "unknowns-partials-100-xid.saved",
        INPUT_PATH "unknowns-partials-100-xzafj.saved",
        INPUT_PATH "unknowns-partials-100-xah.saved",
        INPUT_PATH "unknowns-partials-100-xhy.saved",
        INPUT_PATH "unknowns-partials-100-xzabu.saved",
        INPUT_PATH "unknowns-partials-100-xdz.saved",
        INPUT_PATH "unknowns-partials-100-xtm.saved",
        INPUT_PATH "unknowns-partials-100-xxl.saved",
        INPUT_PATH "unknowns-partials-100-xgb.saved",
        INPUT_PATH "unknowns-partials-100-xgy.saved",
        INPUT_PATH "unknowns-partials-100-xtr.saved",
        INPUT_PATH "unknowns-partials-100-xsh.saved",
        INPUT_PATH "unknowns-partials-100-xwx.saved",
        INPUT_PATH "unknowns-partials-100-xlw.saved",
        INPUT_PATH "unknowns-partials-100-xhl.saved",
        INPUT_PATH "unknowns-partials-100-xvb.saved",
        INPUT_PATH "unknowns-partials-100-xzady.saved",
        INPUT_PATH "unknowns-partials-100-xeh.saved",
        INPUT_PATH "unknowns-partials-100-xuw.saved",
        INPUT_PATH "unknowns-partials-100-xwm.saved",
        INPUT_PATH "unknowns-partials-100-xvl.saved",
        INPUT_PATH "unknowns-partials-100-xcd.saved",
        INPUT_PATH "unknowns-partials-100-xxb.saved",
        INPUT_PATH "unknowns-partials-100-xzacb.saved",
        INPUT_PATH "unknowns-partials-100-xmy.saved",
        INPUT_PATH "unknowns-partials-100-xbp.saved",
        INPUT_PATH "unknowns-partials-100-xnc.saved",
        INPUT_PATH "unknowns-partials-100-xge.saved",
        INPUT_PATH "unknowns-partials-100-xzaay.saved",
        INPUT_PATH "unknowns-partials-100-xzabo.saved",
        INPUT_PATH "unknowns-partials-100-xfj.saved",
        INPUT_PATH "unknowns-partials-100-xzaev.saved",
        INPUT_PATH "unknowns-partials-100-xqb.saved",
        INPUT_PATH "unknowns-partials-100-xyi.saved",
        INPUT_PATH "unknowns-partials-100-xsy.saved",
        INPUT_PATH "unknowns-partials-100-xzahx.saved",
        INPUT_PATH "unknowns-partials-100-xzaey.saved",
        INPUT_PATH "unknowns-partials-100-xzabh.saved",
        INPUT_PATH "unknowns-partials-100-xwi.saved",
        INPUT_PATH "unknowns-partials-100-xzaci.saved",
        INPUT_PATH "unknowns-partials-100-xus.saved",
        INPUT_PATH "unknowns-partials-100-xxw.saved",
        INPUT_PATH "unknowns-partials-100-xzabb.saved",
        INPUT_PATH "unknowns-partials-100-xzaby.saved",
        INPUT_PATH "unknowns-partials-100-xkg.saved",
        INPUT_PATH "unknowns-partials-100-xzaiw.saved",
        INPUT_PATH "unknowns-partials-100-xbn.saved",
        INPUT_PATH "unknowns-partials-100-xym.saved",
        INPUT_PATH "unknowns-partials-100-xrt.saved",
        INPUT_PATH "unknowns-partials-100-xmu.saved",
        INPUT_PATH "unknowns-partials-100-xcy.saved",
        INPUT_PATH "unknowns-partials-100-xof.saved",
        INPUT_PATH "unknowns-partials-100-xjz.saved",
        INPUT_PATH "unknowns-partials-100-xaa.saved",
        INPUT_PATH "unknowns-partials-100-xqd.saved",
        INPUT_PATH "unknowns-partials-100-xlf.saved",
        INPUT_PATH "unknowns-partials-100-xxz.saved",
        INPUT_PATH "unknowns-partials-100-xfm.saved",
        INPUT_PATH "unknowns-partials-100-xrh.saved",
        INPUT_PATH "unknowns-partials-100-xot.saved",
        INPUT_PATH "unknowns-partials-100-xaq.saved",
        INPUT_PATH "unknowns-partials-100-xqi.saved",
        INPUT_PATH "unknowns-partials-100-xnb.saved",
        INPUT_PATH "unknowns-partials-100-xbh.saved",
        INPUT_PATH "unknowns-partials-100-xap.saved",
        INPUT_PATH "unknowns-partials-100-xbc.saved",
        INPUT_PATH "unknowns-partials-100-xkl.saved",
        INPUT_PATH "unknowns-partials-100-xcr.saved",
        INPUT_PATH "unknowns-partials-100-xdv.saved",
        INPUT_PATH "unknowns-partials-100-xpy.saved",
        INPUT_PATH "unknowns-partials-100-xts.saved",
        INPUT_PATH "unknowns-partials-100-xki.saved",
        INPUT_PATH "unknowns-partials-100-xiw.saved",
        INPUT_PATH "unknowns-partials-100-xzaac.saved",
        INPUT_PATH "unknowns-partials-100-xzaep.saved",
        INPUT_PATH "unknowns-partials-100-xpe.saved",
        INPUT_PATH "unknowns-partials-100-xmj.saved",
        INPUT_PATH "unknowns-partials-100-xzaar.saved",
        INPUT_PATH "unknowns-partials-100-xzacs.saved",
        INPUT_PATH "unknowns-partials-100-xdn.saved",
        INPUT_PATH "unknowns-partials-100-xzage.saved",
        INPUT_PATH "unknowns-partials-100-xca.saved",
        INPUT_PATH "unknowns-partials-100-xtw.saved",
        INPUT_PATH "unknowns-partials-100-xac.saved",
        INPUT_PATH "unknowns-partials-100-xqh.saved",
        INPUT_PATH "unknowns-partials-100-xjy.saved",
        INPUT_PATH "unknowns-partials-100-xey.saved",
        INPUT_PATH "unknowns-partials-100-xpj.saved",
        INPUT_PATH "unknowns-partials-100-xzado.saved",
        INPUT_PATH "unknowns-partials-100-xkf.saved",
        INPUT_PATH "unknowns-partials-100-xfv.saved",
        INPUT_PATH "unknowns-partials-100-xzacu.saved",
        INPUT_PATH "unknowns-partials-100-xzaau.saved",
        INPUT_PATH "unknowns-partials-100-xug.saved",
        INPUT_PATH "unknowns-partials-100-xkh.saved",
        INPUT_PATH "unknowns-partials-100-xow.saved",
        INPUT_PATH "unknowns-partials-100-xfn.saved",
        INPUT_PATH "unknowns-partials-100-xxn.saved",
        INPUT_PATH "unknowns-partials-100-xzags.saved",
        INPUT_PATH "unknowns-partials-100-xzafv.saved",
        INPUT_PATH "unknowns-partials-100-xmc.saved",
        INPUT_PATH "unknowns-partials-100-xzabi.saved",
        INPUT_PATH "unknowns-partials-100-xup.saved",
        INPUT_PATH "unknowns-partials-100-xzadp.saved",
        INPUT_PATH "unknowns-partials-100-xfg.saved",
        INPUT_PATH "unknowns-partials-100-xpn.saved",
        INPUT_PATH "unknowns-partials-100-xhr.saved",
        INPUT_PATH "unknowns-partials-100-xov.saved",
        INPUT_PATH "unknowns-partials-100-xny.saved",
        INPUT_PATH "unknowns-partials-100-xwq.saved",
        INPUT_PATH "unknowns-partials-100-xzacn.saved",
        INPUT_PATH "unknowns-partials-100-xtj.saved",
        INPUT_PATH "unknowns-partials-100-xjk.saved",
        INPUT_PATH "unknowns-partials-100-xdc.saved",
        INPUT_PATH "unknowns-partials-100-xbs.saved",
        INPUT_PATH "unknowns-partials-100-xea.saved",
        INPUT_PATH "unknowns-partials-100-xjl.saved",
        INPUT_PATH "unknowns-partials-100-xix.saved",
        INPUT_PATH "unknowns-partials-100-xdk.saved",
        INPUT_PATH "unknowns-partials-100-xco.saved",
        INPUT_PATH "unknowns-partials-100-xhu.saved",
        INPUT_PATH "unknowns-partials-100-xrd.saved",
        INPUT_PATH "unknowns-partials-100-xsk.saved",
        INPUT_PATH "unknowns-partials-100-xai.saved",
        INPUT_PATH "unknowns-partials-100-xkq.saved",
        INPUT_PATH "unknowns-partials-100-xte.saved",
        INPUT_PATH "unknowns-partials-100-xjc.saved",
        INPUT_PATH "unknowns-partials-100-xwo.saved",
        INPUT_PATH "unknowns-partials-100-xzaex.saved",
        INPUT_PATH "unknowns-partials-100-xjh.saved",
        INPUT_PATH "unknowns-partials-100-xzabt.saved",
        INPUT_PATH "unknowns-partials-100-xyl.saved",
        INPUT_PATH "unknowns-partials-100-xyf.saved",
        INPUT_PATH "unknowns-partials-100-xzaiq.saved",
        INPUT_PATH "unknowns-partials-100-xfx.saved",
        INPUT_PATH "unknowns-partials-100-xlh.saved",
        INPUT_PATH "unknowns-partials-100-xqa.saved",
        INPUT_PATH "unknowns-partials-100-xzafx.saved",
        INPUT_PATH "unknowns-partials-100-xdm.saved",
        INPUT_PATH "unknowns-partials-100-xzabl.saved",
        INPUT_PATH "unknowns-partials-100-xzace.saved",
        INPUT_PATH "unknowns-partials-100-xci.saved",
        INPUT_PATH "unknowns-partials-100-xzaew.saved",
        INPUT_PATH "unknowns-partials-100-xzaho.saved",
        INPUT_PATH "unknowns-partials-100-xzaeu.saved",
        INPUT_PATH "unknowns-partials-100-xsd.saved",
        INPUT_PATH "unknowns-partials-100-xvg.saved",
        INPUT_PATH "unknowns-partials-100-xtg.saved",
        INPUT_PATH "unknowns-partials-100-xzagz.saved",
        INPUT_PATH "unknowns-partials-100-xgd.saved",
        INPUT_PATH "unknowns-partials-100-xdf.saved",
        INPUT_PATH "unknowns-partials-100-xtv.saved",
        INPUT_PATH "unknowns-partials-100-xut.saved",
        INPUT_PATH "unknowns-partials-100-xzafh.saved",
        INPUT_PATH "unknowns-partials-100-xpk.saved",
        INPUT_PATH "unknowns-partials-100-xzacy.saved",
        INPUT_PATH "unknowns-partials-100-xnr.saved",
        INPUT_PATH "unknowns-partials-100-xsl.saved",
        INPUT_PATH "unknowns-partials-100-xdi.saved",
        INPUT_PATH "unknowns-partials-100-xhg.saved",
        INPUT_PATH "unknowns-partials-100-xbk.saved",
        INPUT_PATH "unknowns-partials-100-xzadd.saved",
        INPUT_PATH "unknowns-partials-100-xvu.saved",
        INPUT_PATH "unknowns-partials-100-xzafw.saved",
        INPUT_PATH "unknowns-partials-100-xzadq.saved",
        INPUT_PATH "unknowns-partials-100-xzaea.saved",
        INPUT_PATH "unknowns-partials-100-xzahy.saved",
        INPUT_PATH "unknowns-partials-100-xzaeh.saved",
        INPUT_PATH "unknowns-partials-100-xet.saved",
        INPUT_PATH "unknowns-partials-100-xpv.saved",
        INPUT_PATH "unknowns-partials-100-xfy.saved",
        INPUT_PATH "unknowns-partials-100-xzagq.saved",
        INPUT_PATH "unknowns-partials-100-xth.saved",
        INPUT_PATH "unknowns-partials-100-xzahc.saved",
        INPUT_PATH "unknowns-partials-100-xlp.saved",
        INPUT_PATH "unknowns-partials-100-xit.saved",
        INPUT_PATH "unknowns-partials-100-xzadc.saved",
        INPUT_PATH "unknowns-partials-100-xgk.saved",
        INPUT_PATH "unknowns-partials-100-xyp.saved",
        INPUT_PATH "unknowns-partials-100-xmh.saved",
        INPUT_PATH "unknowns-partials-100-xck.saved",
        INPUT_PATH "unknowns-partials-100-xke.saved",
        INPUT_PATH "unknowns-partials-100-xzahb.saved",
        INPUT_PATH "unknowns-partials-100-xzahi.saved",
        INPUT_PATH "unknowns-partials-100-xwj.saved",
        INPUT_PATH "unknowns-partials-100-xda.saved",
        INPUT_PATH "unknowns-partials-100-xfs.saved",
        INPUT_PATH "unknowns-partials-100-xgq.saved",
        INPUT_PATH "unknowns-partials-100-xhz.saved",
        INPUT_PATH "unknowns-partials-100-xwu.saved",
        INPUT_PATH "unknowns-partials-100-xnf.saved",
        INPUT_PATH "unknowns-partials-100-xzabr.saved",
        INPUT_PATH "unknowns-partials-100-xre.saved",
        INPUT_PATH "unknowns-partials-100-xcs.saved",
        INPUT_PATH "unknowns-partials-100-xzahh.saved",
        INPUT_PATH "unknowns-partials-100-xzagf.saved",
        INPUT_PATH "unknowns-partials-100-xrl.saved",
        INPUT_PATH "unknowns-partials-100-xhb.saved",
        INPUT_PATH "unknowns-partials-100-xjp.saved",
        INPUT_PATH "unknowns-partials-100-xjt.saved",
        INPUT_PATH "unknowns-partials-100-xzaed.saved",
        INPUT_PATH "unknowns-partials-100-xzahn.saved",
        INPUT_PATH "unknowns-partials-100-xuh.saved",
        INPUT_PATH "unknowns-partials-100-xzadt.saved",
        INPUT_PATH "unknowns-partials-100-xti.saved",
        INPUT_PATH "unknowns-partials-100-xgn.saved",
        INPUT_PATH "unknowns-partials-100-xvr.saved",
        INPUT_PATH "unknowns-partials-100-xzaas.saved",
        INPUT_PATH "unknowns-partials-100-xcb.saved",
        INPUT_PATH "unknowns-partials-100-xxc.saved",
        INPUT_PATH "unknowns-partials-100-xsr.saved",
        INPUT_PATH "unknowns-partials-100-xme.saved",
        INPUT_PATH "unknowns-partials-100-xye.saved",
        INPUT_PATH "unknowns-partials-100-xyu.saved",
        INPUT_PATH "unknowns-partials-100-xux.saved",
        INPUT_PATH "unknowns-partials-100-xse.saved",
        INPUT_PATH "unknowns-partials-100-xec.saved",
        INPUT_PATH "unknowns-partials-100-xzagn.saved",
        INPUT_PATH "unknowns-partials-100-xzabj.saved",
        INPUT_PATH "unknowns-partials-100-xzaeg.saved",
        INPUT_PATH "unknowns-partials-100-xbe.saved",
        INPUT_PATH "unknowns-partials-100-xzada.saved",
        INPUT_PATH "unknowns-partials-100-xdq.saved",
        INPUT_PATH "unknowns-partials-100-xay.saved",
        INPUT_PATH "unknowns-partials-100-xem.saved",
        INPUT_PATH "unknowns-partials-100-xzaam.saved",
        INPUT_PATH "unknowns-partials-100-xoc.saved",
        INPUT_PATH "unknowns-partials-100-xcl.saved",
        INPUT_PATH "unknowns-partials-100-xgc.saved",
        INPUT_PATH "unknowns-partials-100-xho.saved",
        INPUT_PATH "unknowns-partials-100-xbg.saved",
        INPUT_PATH "unknowns-partials-100-xxo.saved",
        INPUT_PATH "unknowns-partials-100-xrj.saved",
        INPUT_PATH "unknowns-partials-100-xbr.saved",
        INPUT_PATH "unknowns-partials-100-xsi.saved",
        INPUT_PATH "unknowns-partials-100-xzacx.saved",
        INPUT_PATH "unknowns-partials-100-xqy.saved",
        INPUT_PATH "unknowns-partials-100-xph.saved",
        INPUT_PATH "unknowns-partials-100-xzacz.saved",
        INPUT_PATH "unknowns-partials-100-xfw.saved",
        INPUT_PATH "unknowns-partials-100-xpt.saved",
        INPUT_PATH "unknowns-partials-100-xvz.saved",
        INPUT_PATH "unknowns-partials-100-xhj.saved",
        INPUT_PATH "unknowns-partials-100-xzaiu.saved",
        INPUT_PATH "unknowns-partials-100-xjr.saved",
        INPUT_PATH "unknowns-partials-100-xhs.saved",
        INPUT_PATH "unknowns-partials-100-xrz.saved",
        INPUT_PATH "unknowns-partials-100-xzahg.saved",
        INPUT_PATH "unknowns-partials-100-xlx.saved",
        INPUT_PATH "unknowns-partials-100-xzabf.saved",
        INPUT_PATH "unknowns-partials-100-xzagp.saved",
        INPUT_PATH "unknowns-partials-100-xkr.saved",
        INPUT_PATH "unknowns-partials-100-xxq.saved",
        INPUT_PATH "unknowns-partials-100-xcw.saved",
        INPUT_PATH "unknowns-partials-100-xnq.saved",
        INPUT_PATH "unknowns-partials-100-xwf.saved",
        INPUT_PATH "unknowns-partials-100-xzaeo.saved",
        INPUT_PATH "unknowns-partials-100-xfk.saved",
        INPUT_PATH "unknowns-partials-100-xlr.saved",
        INPUT_PATH "unknowns-partials-100-xzagg.saved",
        INPUT_PATH "unknowns-partials-100-xdx.saved",
        INPUT_PATH "unknowns-partials-100-xfb.saved",
        INPUT_PATH "unknowns-partials-100-xcj.saved",
        INPUT_PATH "unknowns-partials-100-xnz.saved",
        INPUT_PATH "unknowns-partials-100-xxx.saved",
        INPUT_PATH "unknowns-partials-100-xrb.saved",
        INPUT_PATH "unknowns-partials-100-xqx.saved",
        INPUT_PATH "unknowns-partials-100-xzadu.saved",
        INPUT_PATH "unknowns-partials-100-xzahq.saved",
        INPUT_PATH "unknowns-partials-100-xpb.saved",
        INPUT_PATH "unknowns-partials-100-xpi.saved",
        INPUT_PATH "unknowns-partials-100-xky.saved",
        INPUT_PATH "unknowns-partials-100-xzaax.saved",
        INPUT_PATH "unknowns-partials-100-xlj.saved",
        INPUT_PATH "unknowns-partials-100-xqe.saved",
        INPUT_PATH "unknowns-partials-100-xdb.saved",
        INPUT_PATH "unknowns-partials-100-xoz.saved",
        INPUT_PATH "unknowns-partials-100-xas.saved",
        INPUT_PATH "unknowns-partials-100-xae.saved",
        INPUT_PATH "unknowns-partials-100-xop.saved",
        INPUT_PATH "unknowns-partials-100-xdg.saved",
        INPUT_PATH "unknowns-partials-100-xpx.saved",
        INPUT_PATH "unknowns-partials-100-xez.saved",
        INPUT_PATH "unknowns-partials-100-xha.saved",
        INPUT_PATH "unknowns-partials-100-xcp.saved",
        INPUT_PATH "unknowns-partials-100-xcx.saved",
        INPUT_PATH "unknowns-partials-100-xfi.saved",
        INPUT_PATH "unknowns-partials-100-xzacv.saved",
        INPUT_PATH "unknowns-partials-100-xmk.saved",
        INPUT_PATH "unknowns-partials-100-xmx.saved",
        INPUT_PATH "unknowns-partials-100-xzaek.saved",
        INPUT_PATH "unknowns-partials-100-xzafi.saved",
        INPUT_PATH "unknowns-partials-100-xto.saved",
        INPUT_PATH "unknowns-partials-100-xvf.saved",
        INPUT_PATH "unknowns-partials-100-xal.saved",
        INPUT_PATH "unknowns-partials-100-xhx.saved",
        INPUT_PATH "unknowns-partials-100-xzabd.saved",
        INPUT_PATH "unknowns-partials-100-xtc.saved",
        INPUT_PATH "unknowns-partials-100-xhf.saved",
        INPUT_PATH "unknowns-partials-100-xzahj.saved",
        INPUT_PATH "unknowns-partials-100-xzahz.saved",
        INPUT_PATH "unknowns-partials-100-xrv.saved",
        INPUT_PATH "unknowns-partials-100-xgr.saved",
        INPUT_PATH "unknowns-partials-100-xzagx.saved",
        INPUT_PATH "unknowns-partials-100-xeq.saved",
        INPUT_PATH "unknowns-partials-100-xzacm.saved",
        INPUT_PATH "unknowns-partials-100-xqf.saved",
        INPUT_PATH "unknowns-partials-100-xzait.saved",
        INPUT_PATH "unknowns-partials-100-xek.saved",
        INPUT_PATH "unknowns-partials-100-xxv.saved",
        INPUT_PATH "unknowns-partials-100-xku.saved",
        INPUT_PATH "unknowns-partials-100-xgs.saved",
        INPUT_PATH "unknowns-partials-100-xui.saved",
        INPUT_PATH "unknowns-partials-100-xrn.saved",
        INPUT_PATH "unknowns-partials-100-xyh.saved",
        INPUT_PATH "unknowns-partials-100-xyv.saved",
        INPUT_PATH "unknowns-partials-100-xim.saved",
        INPUT_PATH "unknowns-partials-100-xcg.saved",
        INPUT_PATH "unknowns-partials-100-xrg.saved",
        INPUT_PATH "unknowns-partials-100-xwk.saved",
        INPUT_PATH "unknowns-partials-100-xxu.saved",
        INPUT_PATH "unknowns-partials-100-xjx.saved",
        INPUT_PATH "unknowns-partials-100-xht.saved",
        INPUT_PATH "unknowns-partials-100-xic.saved",
        INPUT_PATH "unknowns-partials-100-xzaft.saved",
        INPUT_PATH "unknowns-partials-100-xxm.saved",
        INPUT_PATH "unknowns-partials-100-xwb.saved",
        INPUT_PATH "unknowns-partials-100-xzadb.saved",
        INPUT_PATH "unknowns-partials-100-xzabg.saved",
        INPUT_PATH "unknowns-partials-100-xed.saved",
        INPUT_PATH "unknowns-partials-100-xpm.saved",
        INPUT_PATH "unknowns-partials-100-xgf.saved",
        INPUT_PATH "unknowns-partials-100-xir.saved",
        INPUT_PATH "unknowns-partials-100-xmf.saved",
        INPUT_PATH "unknowns-partials-100-xwp.saved",
        INPUT_PATH "unknowns-partials-100-xxh.saved",
        INPUT_PATH "unknowns-partials-100-xzaap.saved",
        INPUT_PATH "unknowns-partials-100-xzabv.saved",
        INPUT_PATH "unknowns-partials-100-xvk.saved",
        INPUT_PATH "unknowns-partials-100-xcv.saved",
        INPUT_PATH "unknowns-partials-100-xns.saved",
        INPUT_PATH "unknowns-partials-100-xzaii.saved",
        INPUT_PATH "unknowns-partials-100-xyz.saved",
        INPUT_PATH "unknowns-partials-100-xvt.saved",
        INPUT_PATH "unknowns-partials-100-xdd.saved",
        INPUT_PATH "unknowns-partials-100-xzahm.saved",
        INPUT_PATH "unknowns-partials-100-xzack.saved",
        INPUT_PATH "unknowns-partials-100-xkp.saved",
        INPUT_PATH "unknowns-partials-100-xzahu.saved",
        INPUT_PATH "unknowns-partials-100-xxr.saved",
        INPUT_PATH "unknowns-partials-100-xzaai.saved",
        INPUT_PATH "unknowns-partials-100-xzadr.saved",
        INPUT_PATH "unknowns-partials-100-xll.saved",
        INPUT_PATH "unknowns-partials-100-xiq.saved",
        INPUT_PATH "unknowns-partials-100-xzaij.saved",
        INPUT_PATH "unknowns-partials-100-xob.saved",
        INPUT_PATH "unknowns-partials-100-xos.saved",
        INPUT_PATH "unknowns-partials-100-xzaef.saved",
        INPUT_PATH "unknowns-partials-100-xqg.saved",
        INPUT_PATH "unknowns-partials-100-xzaik.saved",
        INPUT_PATH "unknowns-partials-100-xdj.saved",
        INPUT_PATH "unknowns-partials-100-xbq.saved",
        INPUT_PATH "unknowns-partials-100-xjb.saved",
        INPUT_PATH "unknowns-partials-100-xpf.saved" } },
    { true,
      ORDER_PATH_FMT "-unknowns-10000",
      49071063344,
      { INPUT_PATH "unknowns-partials-10000-xad.saved",
        INPUT_PATH "unknowns-partials-10000-xab.saved",
        INPUT_PATH "unknowns-partials-10000-xaf.saved",
        INPUT_PATH "unknowns-partials-10000-xag.saved",
        INPUT_PATH "unknowns-partials-10000-xah.saved",
        INPUT_PATH "unknowns-partials-10000-xaa.saved",
        INPUT_PATH "unknowns-partials-10000-xac.saved",
        INPUT_PATH "unknowns-partials-10000-xai.saved",
        INPUT_PATH "unknowns-partials-10000-xae.saved" } },
    { true,
      ORDER_PATH_FMT "-unknowns-25000",
      49071139014,
      { INPUT_PATH "unknowns-partials-25000-xad.saved",
        INPUT_PATH "unknowns-partials-25000-xab.saved",
        INPUT_PATH "unknowns-partials-25000-xaa.saved",
        INPUT_PATH "unknowns-partials-25000-xac.saved" } },
    { true,
      ORDER_PATH_FMT "-unknowns-1000",
      49068467577,
      { INPUT_PATH "unknowns-partials-1000-xct.saved",
        INPUT_PATH "unknowns-partials-1000-xam.saved",
        INPUT_PATH "unknowns-partials-1000-xcm.saved",
        INPUT_PATH "unknowns-partials-1000-xce.saved",
        INPUT_PATH "unknowns-partials-1000-xbj.saved",
        INPUT_PATH "unknowns-partials-1000-xcn.saved",
        INPUT_PATH "unknowns-partials-1000-xak.saved",
        INPUT_PATH "unknowns-partials-1000-xad.saved",
        INPUT_PATH "unknowns-partials-1000-xav.saved",
        INPUT_PATH "unknowns-partials-1000-xbv.saved",
        INPUT_PATH "unknowns-partials-1000-xde.saved",
        INPUT_PATH "unknowns-partials-1000-xaj.saved",
        INPUT_PATH "unknowns-partials-1000-xbo.saved",
        INPUT_PATH "unknowns-partials-1000-xat.saved",
        INPUT_PATH "unknowns-partials-1000-xcc.saved",
        INPUT_PATH "unknowns-partials-1000-xbm.saved",
        INPUT_PATH "unknowns-partials-1000-xch.saved",
        INPUT_PATH "unknowns-partials-1000-xbt.saved",
        INPUT_PATH "unknowns-partials-1000-xan.saved",
        INPUT_PATH "unknowns-partials-1000-xbx.saved",
        INPUT_PATH "unknowns-partials-1000-xbz.saved",
        INPUT_PATH "unknowns-partials-1000-xbf.saved",
        INPUT_PATH "unknowns-partials-1000-xbw.saved",
        INPUT_PATH "unknowns-partials-1000-xau.saved",
        INPUT_PATH "unknowns-partials-1000-xcu.saved",
        INPUT_PATH "unknowns-partials-1000-xab.saved",
        INPUT_PATH "unknowns-partials-1000-xao.saved",
        INPUT_PATH "unknowns-partials-1000-xcz.saved",
        INPUT_PATH "unknowns-partials-1000-xbb.saved",
        INPUT_PATH "unknowns-partials-1000-xdh.saved",
        INPUT_PATH "unknowns-partials-1000-xar.saved",
        INPUT_PATH "unknowns-partials-1000-xaf.saved",
        INPUT_PATH "unknowns-partials-1000-xba.saved",
        INPUT_PATH "unknowns-partials-1000-xcq.saved",
        INPUT_PATH "unknowns-partials-1000-xaw.saved",
        INPUT_PATH "unknowns-partials-1000-xbu.saved",
        INPUT_PATH "unknowns-partials-1000-xbl.saved",
        INPUT_PATH "unknowns-partials-1000-xag.saved",
        INPUT_PATH "unknowns-partials-1000-xbi.saved",
        INPUT_PATH "unknowns-partials-1000-xbd.saved",
        INPUT_PATH "unknowns-partials-1000-xcf.saved",
        INPUT_PATH "unknowns-partials-1000-xby.saved",
        INPUT_PATH "unknowns-partials-1000-xaz.saved",
        INPUT_PATH "unknowns-partials-1000-xax.saved",
        INPUT_PATH "unknowns-partials-1000-xah.saved",
        INPUT_PATH "unknowns-partials-1000-xcd.saved",
        INPUT_PATH "unknowns-partials-1000-xbp.saved",
        INPUT_PATH "unknowns-partials-1000-xbn.saved",
        INPUT_PATH "unknowns-partials-1000-xcy.saved",
        INPUT_PATH "unknowns-partials-1000-xaa.saved",
        INPUT_PATH "unknowns-partials-1000-xaq.saved",
        INPUT_PATH "unknowns-partials-1000-xbh.saved",
        INPUT_PATH "unknowns-partials-1000-xap.saved",
        INPUT_PATH "unknowns-partials-1000-xbc.saved",
        INPUT_PATH "unknowns-partials-1000-xcr.saved",
        INPUT_PATH "unknowns-partials-1000-xca.saved",
        INPUT_PATH "unknowns-partials-1000-xac.saved",
        INPUT_PATH "unknowns-partials-1000-xdc.saved",
        INPUT_PATH "unknowns-partials-1000-xbs.saved",
        INPUT_PATH "unknowns-partials-1000-xdk.saved",
        INPUT_PATH "unknowns-partials-1000-xco.saved",
        INPUT_PATH "unknowns-partials-1000-xai.saved",
        INPUT_PATH "unknowns-partials-1000-xci.saved",
        INPUT_PATH "unknowns-partials-1000-xdf.saved",
        INPUT_PATH "unknowns-partials-1000-xdi.saved",
        INPUT_PATH "unknowns-partials-1000-xbk.saved",
        INPUT_PATH "unknowns-partials-1000-xck.saved",
        INPUT_PATH "unknowns-partials-1000-xda.saved",
        INPUT_PATH "unknowns-partials-1000-xcs.saved",
        INPUT_PATH "unknowns-partials-1000-xcb.saved",
        INPUT_PATH "unknowns-partials-1000-xbe.saved",
        INPUT_PATH "unknowns-partials-1000-xay.saved",
        INPUT_PATH "unknowns-partials-1000-xcl.saved",
        INPUT_PATH "unknowns-partials-1000-xbg.saved",
        INPUT_PATH "unknowns-partials-1000-xbr.saved",
        INPUT_PATH "unknowns-partials-1000-xcw.saved",
        INPUT_PATH "unknowns-partials-1000-xcj.saved",
        INPUT_PATH "unknowns-partials-1000-xdb.saved",
        INPUT_PATH "unknowns-partials-1000-xas.saved",
        INPUT_PATH "unknowns-partials-1000-xae.saved",
        INPUT_PATH "unknowns-partials-1000-xdg.saved",
        INPUT_PATH "unknowns-partials-1000-xcp.saved",
        INPUT_PATH "unknowns-partials-1000-xcx.saved",
        INPUT_PATH "unknowns-partials-1000-xal.saved",
        INPUT_PATH "unknowns-partials-1000-xcg.saved",
        INPUT_PATH "unknowns-partials-1000-xcv.saved",
        INPUT_PATH "unknowns-partials-1000-xdd.saved",
        INPUT_PATH "unknowns-partials-1000-xdj.saved",
        INPUT_PATH "unknowns-partials-1000-xbq.saved" } },
    { true,
      ORDER_PATH_FMT "-unknowns-5000",
      49070951830,
      { INPUT_PATH "unknowns-partials-5000-xam.saved",
        INPUT_PATH "unknowns-partials-5000-xak.saved",
        INPUT_PATH "unknowns-partials-5000-xad.saved",
        INPUT_PATH "unknowns-partials-5000-xaj.saved",
        INPUT_PATH "unknowns-partials-5000-xan.saved",
        INPUT_PATH "unknowns-partials-5000-xab.saved",
        INPUT_PATH "unknowns-partials-5000-xao.saved",
        INPUT_PATH "unknowns-partials-5000-xar.saved",
        INPUT_PATH "unknowns-partials-5000-xaf.saved",
        INPUT_PATH "unknowns-partials-5000-xag.saved",
        INPUT_PATH "unknowns-partials-5000-xah.saved",
        INPUT_PATH "unknowns-partials-5000-xaa.saved",
        INPUT_PATH "unknowns-partials-5000-xaq.saved",
        INPUT_PATH "unknowns-partials-5000-xap.saved",
        INPUT_PATH "unknowns-partials-5000-xac.saved",
        INPUT_PATH "unknowns-partials-5000-xai.saved",
        INPUT_PATH "unknowns-partials-5000-xae.saved",
        INPUT_PATH "unknowns-partials-5000-xal.saved" } },
    { true,
      ORDER_PATH_FMT "-unknowns-50000",
      49071176601,
      { INPUT_PATH "unknowns-partials-50000-xab.saved",
        INPUT_PATH "unknowns-partials-50000-xaa.saved" } },
};
TLO_REENABLE_WEXIT_TIME_DESTRUCTORS


// NOLINTEND(cppcoreguidelines-avoid-c-arrays,hicpp-avoid-c-arrays,modernize-avoid-c-arrays,bugprone-suspicious-missing-comma)
TEST(perf, save_no_rescale) {
    init_env();
    tlo::vec_t<std::string_view> ss_files_in = {
        INPUT_PATH "cross-build-id.0.saved", INPUT_PATH "cross-build-id.1.saved"
    };


    std::array<tlo::sym::sym_state_t, 2>              ss_in          = { {} };
    std::array<tlo::vec_t<tlo::perf::perf_func_t>, 2> funcs_in       = { {} };
    std::array<tlo::vec_t<tlo::perf::perf_edge_t>, 2> edges_in       = { {} };
    std::array<tlo::perf::perf_state_scaling_t, 2> scaling_todos_out = { {} };
    for (unsigned i = 0; i < 2; ++i) {
        const tlo::perf::perf_state_reloader_t reloader_in{ &ss_in[i] };
        scaling_todos_out[i].set_force_no_scale();
        ASSERT_TRUE(reloader_in.reload_state(
            ss_files_in[i], &funcs_in[i], &edges_in[i], &scaling_todos_out[i]));
        ASSERT_FALSE(scaling_todos_out[i].did_scale_any());
    }

    // NOLINTNEXTLINE(*magic*)
    std::array<std::array<char, 256>, 2> tmpfiles_out;
    std::array<int, 2>                   fds_out = { {} };
    for (unsigned i = 0; i < 2; ++i) {
        fds_out[i] = tlo::file_ops::new_tmpfile(&tmpfiles_out[i]);
        ASSERT_GT(fds_out[i], 0);
        const tlo::perf::perf_state_saver_t saver{ &ss_in[i] };
        ASSERT_TRUE(saver.save_state(tmpfiles_out[i].data(), &funcs_in[i],
                                     &edges_in[i], &scaling_todos_out[i]));
    }

    const tlo::vec_t<std::string_view> ss_files_out = {
        tmpfiles_out[0].data(), tmpfiles_out[1].data()
    };

    tlo::sym::sym_state_t              ss_out{};
    tlo::vec_t<tlo::perf::perf_func_t> funcs_out{};
    tlo::vec_t<tlo::perf::perf_edge_t> edges_out{};
    {
        tlo::perf::perf_state_scaling_t        scaling_todo{};
        const tlo::perf::perf_state_reloader_t reloader_out{ &ss_out };
        scaling_todo.set_force_no_scale();
        ASSERT_TRUE(reloader_out.reload_state(&ss_files_out, &funcs_out,
                                              &edges_out, &scaling_todo));
        ASSERT_FALSE(scaling_todo.did_scale_any());
    }

    close(fds_out[0]);
    close(fds_out[1]);

    tlo::perf::perf_func_stats_t check_func_stats{};
    tlo::perf::perf_edge_stats_t check_edge_stats{};
    uint64_t                     total_size = 0;
    for (const auto & pfunc : funcs_out) {
        ASSERT_TRUE(pfunc.valid());
        check_func_stats.add(pfunc.stats());
        total_size += pfunc.func_clump_->size();
    }

    for (const auto & pedge : edges_out) {
        ASSERT_TRUE(pedge.valid());
        check_edge_stats.add(pedge.stats());
        total_size += pedge.from_->size();
        total_size += pedge.to_->size();
    }

    EXPECT_EQ(total_size, 48244763832UL);
    EXPECT_EQ(check_func_stats.num_samples_,
              static_cast<tlo::perf::psample_val_t>(1471302UL))
        << check_func_stats.num_samples_;
    EXPECT_EQ(check_func_stats.num_tracked_br_samples_in_,
              static_cast<tlo::perf::psample_val_t>(8906322UL))
        << check_func_stats.num_tracked_br_samples_in_;
    EXPECT_EQ(check_func_stats.num_tracked_br_samples_out_,
              static_cast<tlo::perf::psample_val_t>(8906322UL))
        << check_func_stats.num_tracked_br_samples_out_;
    EXPECT_EQ(check_func_stats.num_br_samples_in_,
              static_cast<tlo::perf::psample_val_t>(48444158UL))
        << check_func_stats.num_br_samples_in_;
    EXPECT_EQ(check_func_stats.num_br_samples_out_,
              static_cast<tlo::perf::psample_val_t>(48444158UL))
        << check_func_stats.num_br_samples_out_;
    EXPECT_EQ(check_edge_stats.num_edges_,
              static_cast<tlo::perf::psample_val_t>(8906322UL))
        << check_edge_stats.num_edges_;
}

TEST(perf, save_no_rescale_of_scaled) {
    init_env();
    tlo::vec_t<std::string_view> ss_files_in = {
        INPUT_PATH "cross-build-id.0.saved", INPUT_PATH "cross-build-id.1.saved"
    };
    static constexpr std::array<std::pair<double, double>, 4> k_confs = { {
        std::pair<double, double>{ 2.0, 2.0 },
        std::pair<double, double>{ 0.0, 2.0 },
        std::pair<double, double>{ 2.0, 0.0 },
        std::pair<double, double>{ 0.0, 0.0 },
    } };
    for (unsigned att = 0; att <= 3; ++att) {
        std::array<tlo::sym::sym_state_t, 2>              ss_in    = { {} };
        std::array<tlo::vec_t<tlo::perf::perf_func_t>, 2> funcs_in = { {} };
        std::array<tlo::vec_t<tlo::perf::perf_edge_t>, 2> edges_in = { {} };
        std::array<tlo::perf::perf_state_scaling_t, 2>    scaling_todos_out = {
            {}
        };
        for (unsigned i = 0; i < 2; ++i) {
            const tlo::perf::perf_state_reloader_t reloader_in{ &ss_in[i] };
            if (((i + 1) & att) != 0 && att != 3) {
                scaling_todos_out[i].set_force_no_scale();
            }
            ASSERT_TRUE(reloader_in.reload_state(ss_files_in[i], &funcs_in[i],
                                                 &edges_in[i],
                                                 &scaling_todos_out[i]));
            if (((i + 1) & att) != 0 && att != 3) {
                ASSERT_FALSE(scaling_todos_out[i].did_scale_any());
            }
            else {
                ASSERT_TRUE(scaling_todos_out[i].did_scale());
            }
        }

        // NOLINTNEXTLINE(*magic*)
        std::array<std::array<char, 256>, 2> tmpfiles_out;
        std::array<int, 2>                   fds_out = { { 0, 0 } };
        for (unsigned i = 0; i < 2; ++i) {
            fds_out[i] = tlo::file_ops::new_tmpfile(&tmpfiles_out[i]);
            ASSERT_GT(fds_out[i], 0);
            const tlo::perf::perf_state_saver_t saver{ &ss_in[i] };

            ASSERT_FALSE(scaling_todos_out[i].set_add_scale(
                0.0, tlo::perf::perf_state_scaling_t::k_func_only));
            ASSERT_FALSE(scaling_todos_out[i].set_add_scale(
                -2.0, tlo::perf::perf_state_scaling_t::k_edge_only));

            if (k_confs[att].first != 0.0) {
                scaling_todos_out[i].set_add_scale(
                    k_confs[att].first,
                    tlo::perf::perf_state_scaling_t::k_func_only);
            }
            if (k_confs[att].second != 0.0) {
                scaling_todos_out[i].set_add_scale(
                    k_confs[att].second,
                    tlo::perf::perf_state_scaling_t::k_edge_only);
            }

            ASSERT_TRUE(saver.save_state(tmpfiles_out[i].data(), &funcs_in[i],
                                         &edges_in[i], &scaling_todos_out[i]));
        }

        const tlo::vec_t<std::string_view> ss_files_out = {
            tmpfiles_out[0].data(), tmpfiles_out[1].data()
        };

        tlo::sym::sym_state_t              ss_out{};
        tlo::vec_t<tlo::perf::perf_func_t> funcs_out{};
        tlo::vec_t<tlo::perf::perf_edge_t> edges_out{};
        {
            tlo::perf::perf_state_scaling_t        scaling_todo{};
            const tlo::perf::perf_state_reloader_t reloader_out{ &ss_out };
#ifndef TLO_SANITIZED
            // This causes a leak.
            scaling_todo.set_no_scale();
            ASSERT_FALSE(reloader_out.reload_state(&ss_files_out, &funcs_out,
                                                   &edges_out, &scaling_todo));
#endif
            scaling_todo.set_force_no_scale();
            ASSERT_TRUE(reloader_out.reload_state(&ss_files_out, &funcs_out,
                                                  &edges_out, &scaling_todo));
            ASSERT_TRUE(scaling_todo.did_scale_any()) << att;
        }

        close(fds_out[0]);
        close(fds_out[1]);

        tlo::perf::perf_func_stats_t check_func_stats{};
        tlo::perf::perf_edge_stats_t check_edge_stats{};
        uint64_t                     total_size = 0;
        for (const auto & pfunc : funcs_out) {
            ASSERT_TRUE(pfunc.valid());
            check_func_stats.add(pfunc.stats());
            total_size += pfunc.func_clump_->size();
        }

        for (const auto & pedge : edges_out) {
            ASSERT_TRUE(pedge.valid());
            check_edge_stats.add(pedge.stats());
            total_size += pedge.from_->size();
            total_size += pedge.to_->size();
        }

        EXPECT_EQ(total_size, 48244763832UL);
        tlo::perf::psample_val_t expec_nsamples = 0.0;
        tlo::perf::psample_val_t expec_nedges   = 0.0;
        switch (att) {
            case 3:
            case 0:
                expec_nsamples = 2 * tlo::perf::k_func_scale_point;
                expec_nedges   = 2 * tlo::perf::k_edge_scale_point;
                break;
                // NOLINTBEGIN(*magic*)
            case 1:
                expec_nsamples =
                    static_cast<tlo::perf::psample_val_t>(1875555UL);
                expec_nedges =
                    static_cast<tlo::perf::psample_val_t>(1078721200UL);
                break;

            case 2:
                expec_nsamples =
                    static_cast<tlo::perf::psample_val_t>(1692899UL);
                expec_nedges =
                    static_cast<tlo::perf::psample_val_t>(1077668770UL);
                break;
                // NOLINTEND(*magic*)
            default:
                ASSERT_TRUE(false);
        }

        EXPECT_NEAR(check_func_stats.num_samples_, expec_nsamples, .99)
            << check_func_stats.num_samples_;
        EXPECT_EQ(check_func_stats.num_tracked_br_samples_in_,
                  static_cast<tlo::perf::psample_val_t>(8906322UL))
            << check_func_stats.num_tracked_br_samples_in_;
        EXPECT_EQ(check_func_stats.num_tracked_br_samples_out_,
                  static_cast<tlo::perf::psample_val_t>(8906322UL))
            << check_func_stats.num_tracked_br_samples_out_;
        EXPECT_EQ(check_func_stats.num_br_samples_in_,
                  static_cast<tlo::perf::psample_val_t>(48444158UL))
            << check_func_stats.num_br_samples_in_;
        EXPECT_EQ(check_func_stats.num_br_samples_out_,
                  static_cast<tlo::perf::psample_val_t>(48444158UL))
            << check_func_stats.num_br_samples_out_;
        EXPECT_NEAR(check_edge_stats.num_edges_, expec_nedges, .99)
            << check_edge_stats.num_edges_;
    }
}


TEST(perf, save_rescale_both) {
    init_env();
    tlo::vec_t<std::string_view> ss_files_in = {
        INPUT_PATH "cross-build-id.0.saved", INPUT_PATH "cross-build-id.1.saved"
    };
    static constexpr std::array<std::pair<double, double>, 16> k_confs = { {
        std::pair<double, double>{ 2.0, 2.0 },
        std::pair<double, double>{ 0.0, 2.0 },
        std::pair<double, double>{ 2.0, 0.0 },
        std::pair<double, double>{ 0.0, 0.0 },
        std::pair<double, double>{ 2.0, 2.0 },
        std::pair<double, double>{ 0.0, 2.0 },
        std::pair<double, double>{ 2.0, 0.0 },
        std::pair<double, double>{ 0.0, 0.0 },
        std::pair<double, double>{ 2.0, 2.0 },
        std::pair<double, double>{ 2.0, 2.0 },
        std::pair<double, double>{ 2.0, 2.0 },
        std::pair<double, double>{ 2.0, 2.0 },
        std::pair<double, double>{ 2.0, 2.0 },
        std::pair<double, double>{ 2.0, 2.0 },
        std::pair<double, double>{ 2.0, 2.0 },
        std::pair<double, double>{ 2.0, 2.0 },
    } };
    // NOLINTNEXTLINE(*magic*)
    for (unsigned att = 0; att < 16; ++att) {
        std::array<tlo::sym::sym_state_t, 2>              ss_in    = { {} };
        std::array<tlo::vec_t<tlo::perf::perf_func_t>, 2> funcs_in = { {} };
        std::array<tlo::vec_t<tlo::perf::perf_edge_t>, 2> edges_in = { {} };
        std::array<tlo::perf::perf_state_scaling_t, 2>    scaling_todos_out = {
            {}
        };
        for (unsigned i = 0; i < 2; ++i) {
            const tlo::perf::perf_state_reloader_t reloader_in{ &ss_in[i] };
            scaling_todos_out[i].set_force_no_scale();
            ASSERT_TRUE(reloader_in.reload_state(ss_files_in[i], &funcs_in[i],
                                                 &edges_in[i],
                                                 &scaling_todos_out[i]));
            ASSERT_FALSE(scaling_todos_out[i].did_scale_any());
        }

        // NOLINTNEXTLINE(*magic*)
        std::array<std::array<char, 256>, 2> tmpfiles_out;
        std::array<int, 2>                   fds_out = { { 0, 0 } };
        for (unsigned i = 0; i < 2; ++i) {
            fds_out[i] = tlo::file_ops::new_tmpfile(&tmpfiles_out[i]);
            ASSERT_GT(fds_out[i], 0);
            const tlo::perf::perf_state_saver_t saver{ &ss_in[i] };

            ASSERT_FALSE(scaling_todos_out[i].set_add_scale(
                0.0, tlo::perf::perf_state_scaling_t::k_func_only));
            ASSERT_FALSE(scaling_todos_out[i].set_add_scale(
                -2.0, tlo::perf::perf_state_scaling_t::k_edge_only));

            ASSERT_EQ(scaling_todos_out[i].added_edge_scale(), 0.0);
            ASSERT_EQ(scaling_todos_out[i].added_func_scale(), 0.0);

            if (k_confs[att % k_confs.size()].first != 0.0) {
                scaling_todos_out[i].set_add_scale(
                    k_confs[att % k_confs.size()].first,
                    tlo::perf::perf_state_scaling_t::k_func_only);
            }
            if (k_confs[att % k_confs.size()].second != 0.0) {
                scaling_todos_out[i].set_add_scale(
                    k_confs[att % k_confs.size()].second,
                    tlo::perf::perf_state_scaling_t::k_edge_only);
            }

            ASSERT_TRUE(saver.save_state(tmpfiles_out[i].data(), &funcs_in[i],
                                         &edges_in[i], &scaling_todos_out[i]));
        }

        const tlo::vec_t<std::string_view> ss_files_out = {
            tmpfiles_out[0].data(), tmpfiles_out[1].data()
        };

        tlo::sym::sym_state_t              ss_out{};
        tlo::vec_t<tlo::perf::perf_func_t> funcs_out{};
        tlo::vec_t<tlo::perf::perf_edge_t> edges_out{};
        {
            tlo::perf::perf_state_scaling_t        scaling_todo{};
            const tlo::perf::perf_state_reloader_t reloader_out{ &ss_out };
            // NOLINTNEXTLINE(*magic*)
            if ((att & 8U) == 0) {
                scaling_todo.set_use_local_scale();
            }
            else {
                switch (att & 1U) {
                    case 0:
                        scaling_todo.set_use_local_scale(
                            tlo::perf::perf_state_scaling_t::k_func_only);
                        break;
                    case 1:
                        scaling_todo.set_use_local_scale(
                            tlo::perf::perf_state_scaling_t::k_edge_only);
                        break;
                    default:
                        ASSERT_FALSE(true);
                }
            }
            // NOLINTNEXTLINE(*magic*)
            if ((att & 4U) == 0) {
                scaling_todo.set_no_scale();
            }
            ASSERT_TRUE(reloader_out.reload_state(&ss_files_out, &funcs_out,
                                                  &edges_out, &scaling_todo));
            // NOLINTNEXTLINE(*magic*)
            if ((att & 4U) == 0) {
                ASSERT_FALSE(scaling_todo.did_scale_any()) << att;
            }
            else {
                ASSERT_TRUE(scaling_todo.did_scale_any()) << att;
            }
        }


        tlo::perf::perf_func_stats_t check_func_stats{};
        tlo::perf::perf_edge_stats_t check_edge_stats{};
        uint64_t                     total_size = 0;
        for (const auto & pfunc : funcs_out) {
            ASSERT_TRUE(pfunc.valid());
            check_func_stats.add(pfunc.stats());
            total_size += pfunc.func_clump_->size();
        }

        for (const auto & pedge : edges_out) {
            ASSERT_TRUE(pedge.valid());
            check_edge_stats.add(pedge.stats());
            total_size += pedge.from_->size();
            total_size += pedge.to_->size();
        }

        EXPECT_EQ(total_size, 48244763832UL);
        tlo::perf::psample_val_t expec_nsamples, expec_nedges;
        expec_nsamples = 0.0;
        expec_nedges   = 0.0;
        // NOLINTBEGIN(*magic*)
        switch (att) {
            case 0:
                expec_nsamples = static_cast<tlo::perf::psample_val_t>(2942604);
                expec_nedges = static_cast<tlo::perf::psample_val_t>(17812644);
                break;
            case 1:
                expec_nsamples = static_cast<tlo::perf::psample_val_t>(1471302);
                expec_nedges = static_cast<tlo::perf::psample_val_t>(17812644);
                break;
            case 2:
                expec_nsamples = static_cast<tlo::perf::psample_val_t>(2942604);
                expec_nedges   = static_cast<tlo::perf::psample_val_t>(8906322);
                break;
            case 3:
                expec_nsamples = static_cast<tlo::perf::psample_val_t>(1471302);
                expec_nedges   = static_cast<tlo::perf::psample_val_t>(8906322);
                break;
            case 4:
            case 5:
            case 6:
            case 7:
                expec_nsamples =
                    2 * tlo::perf::k_func_scale_point *
                    std::max(k_confs[att % k_confs.size()].first, 1.0);
                expec_nedges =
                    2 * tlo::perf::k_edge_scale_point *
                    std::max(k_confs[att % k_confs.size()].second, 1.0);
                break;
            case 8:
                expec_nsamples =
                    static_cast<tlo::perf::psample_val_t>(2942604UL);
                expec_nedges = static_cast<tlo::perf::psample_val_t>(8906322UL);
                break;
            case 9:
                expec_nsamples =
                    static_cast<tlo::perf::psample_val_t>(1471302UL);
                expec_nedges =
                    static_cast<tlo::perf::psample_val_t>(17812644UL);
                break;
            case 10:
                expec_nsamples =
                    static_cast<tlo::perf::psample_val_t>(2942604UL);
                expec_nedges = static_cast<tlo::perf::psample_val_t>(8906322UL);
                break;
            case 11:
                expec_nsamples =
                    static_cast<tlo::perf::psample_val_t>(1471302UL);
                expec_nedges =
                    static_cast<tlo::perf::psample_val_t>(17812644UL);
                break;
            case 12:
                expec_nsamples =
                    static_cast<tlo::perf::psample_val_t>(4194304UL);
                expec_nedges =
                    static_cast<tlo::perf::psample_val_t>(2147483648UL);
                break;
            case 13:
                expec_nsamples =
                    static_cast<tlo::perf::psample_val_t>(2097152UL);
                expec_nedges =
                    static_cast<tlo::perf::psample_val_t>(4294967296UL);
                break;
            case 14:
                expec_nsamples =
                    static_cast<tlo::perf::psample_val_t>(4194304UL);
                expec_nedges =
                    static_cast<tlo::perf::psample_val_t>(2147483648UL);
                break;
            case 15:
                expec_nsamples =
                    static_cast<tlo::perf::psample_val_t>(2097152UL);
                expec_nedges =
                    static_cast<tlo::perf::psample_val_t>(4294967296UL);
                break;
            default:
                ASSERT_FALSE(true);
                break;
        }
        // NOLINTEND(*magic*)

        EXPECT_NEAR(check_func_stats.num_samples_, expec_nsamples, .99)
            << check_func_stats.num_samples_;
        EXPECT_EQ(check_func_stats.num_tracked_br_samples_in_,
                  static_cast<tlo::perf::psample_val_t>(8906322UL))
            << check_func_stats.num_tracked_br_samples_in_;
        EXPECT_EQ(check_func_stats.num_tracked_br_samples_out_,
                  static_cast<tlo::perf::psample_val_t>(8906322UL))
            << check_func_stats.num_tracked_br_samples_out_;
        EXPECT_EQ(check_func_stats.num_br_samples_in_,
                  static_cast<tlo::perf::psample_val_t>(48444158UL))
            << check_func_stats.num_br_samples_in_;
        EXPECT_EQ(check_func_stats.num_br_samples_out_,
                  static_cast<tlo::perf::psample_val_t>(48444158UL))
            << check_func_stats.num_br_samples_out_;
        EXPECT_NEAR(check_edge_stats.num_edges_, expec_nedges, .99)
            << check_edge_stats.num_edges_;

        close(fds_out[0]);
        close(fds_out[1]);
    }
}


TEST(perf, save_rescale_one) {
    init_env();
    tlo::vec_t<std::string_view> ss_files_in = {
        INPUT_PATH "cross-build-id.0.saved", INPUT_PATH "cross-build-id.1.saved"
    };
    static constexpr std::array<std::pair<double, double>, 4> k_confs = { {
        std::pair<double, double>{ 2.0, 2.0 },
        std::pair<double, double>{ 0.0, 2.0 },
        std::pair<double, double>{ 2.0, 0.0 },
        std::pair<double, double>{ 0.0, 0.0 },
    } };
    // NOLINTNEXTLINE(*magic*)
    for (unsigned att = 0; att < 8; ++att) {
        std::array<tlo::sym::sym_state_t, 2>              ss_in    = { {} };
        std::array<tlo::vec_t<tlo::perf::perf_func_t>, 2> funcs_in = { {} };
        std::array<tlo::vec_t<tlo::perf::perf_edge_t>, 2> edges_in = { {} };
        std::array<tlo::perf::perf_state_scaling_t, 2>    scaling_todos_out = {
            {}
        };
        for (unsigned i = 0; i < 2; ++i) {
            const tlo::perf::perf_state_reloader_t reloader_in{ &ss_in[i] };
            scaling_todos_out[i].set_force_no_scale();
            ASSERT_TRUE(reloader_in.reload_state(ss_files_in[i], &funcs_in[i],
                                                 &edges_in[i],
                                                 &scaling_todos_out[i]));
            ASSERT_FALSE(scaling_todos_out[i].did_scale_any());
        }

        // NOLINTNEXTLINE(*magic*)
        std::array<std::array<char, 256>, 2> tmpfiles_out;
        std::array<int, 2>                   fds_out = { {} };
        for (unsigned i = 0; i < 2; ++i) {
            fds_out[i] = tlo::file_ops::new_tmpfile(&tmpfiles_out[i]);
            ASSERT_GT(fds_out[i], 0);
            const tlo::perf::perf_state_saver_t saver{ &ss_in[i] };

            ASSERT_FALSE(scaling_todos_out[i].set_add_scale(
                0.0, tlo::perf::perf_state_scaling_t::k_func_only));
            ASSERT_FALSE(scaling_todos_out[i].set_add_scale(
                -2.0, tlo::perf::perf_state_scaling_t::k_edge_only));

            ASSERT_EQ(scaling_todos_out[i].added_edge_scale(), 0.0);
            ASSERT_EQ(scaling_todos_out[i].added_func_scale(), 0.0);

            if (((i + 1) & att) == 0) {
                if (k_confs[att % k_confs.size()].first != 0.0) {
                    scaling_todos_out[i].set_add_scale(
                        k_confs[att % k_confs.size()].first,
                        tlo::perf::perf_state_scaling_t::k_func_only);
                }
                if (k_confs[att % k_confs.size()].second != 0.0) {
                    scaling_todos_out[i].set_add_scale(
                        k_confs[att % k_confs.size()].second,
                        tlo::perf::perf_state_scaling_t::k_edge_only);
                }
            }

            ASSERT_TRUE(saver.save_state(tmpfiles_out[i].data(), &funcs_in[i],
                                         &edges_in[i], &scaling_todos_out[i]));
        }

        const tlo::vec_t<std::string_view> ss_files_out = {
            tmpfiles_out[0].data(), tmpfiles_out[1].data()
        };

        tlo::sym::sym_state_t              ss_out{};
        tlo::vec_t<tlo::perf::perf_func_t> funcs_out{};
        tlo::vec_t<tlo::perf::perf_edge_t> edges_out{};
        {
            tlo::perf::perf_state_scaling_t        scaling_todo{};
            const tlo::perf::perf_state_reloader_t reloader_out{ &ss_out };
            scaling_todo.set_use_local_scale();
            if (att < 4) {
                scaling_todo.set_no_scale();
            }
            ASSERT_TRUE(reloader_out.reload_state(&ss_files_out, &funcs_out,
                                                  &edges_out, &scaling_todo));
            if (att < 4) {
                ASSERT_FALSE(scaling_todo.did_scale_any()) << att;
            }
            else {
                ASSERT_TRUE(scaling_todo.did_scale_any()) << att;
            }
        }


        tlo::perf::perf_func_stats_t check_func_stats{};
        tlo::perf::perf_edge_stats_t check_edge_stats{};
        uint64_t                     total_size = 0;
        for (const auto & pfunc : funcs_out) {
            ASSERT_TRUE(pfunc.valid());
            check_func_stats.add(pfunc.stats());
            total_size += pfunc.func_clump_->size();
        }

        for (const auto & pedge : edges_out) {
            ASSERT_TRUE(pedge.valid());
            check_edge_stats.add(pedge.stats());
            total_size += pedge.from_->size();
            total_size += pedge.to_->size();
        }

        EXPECT_EQ(total_size, 48244763832UL);
        tlo::perf::psample_val_t expec_nsamples, expec_nedges;
        expec_nsamples = 0.0;
        expec_nedges   = 0.0;


        switch (att) {
                // NOLINTBEGIN(*magic*)
            case 0:
                expec_nsamples = static_cast<tlo::perf::psample_val_t>(2942604);
                expec_nedges = static_cast<tlo::perf::psample_val_t>(17812644);
                break;
            case 1:
                expec_nsamples = static_cast<tlo::perf::psample_val_t>(1471302);
                expec_nedges = static_cast<tlo::perf::psample_val_t>(12833268);
                break;
            case 2:
                expec_nsamples = static_cast<tlo::perf::psample_val_t>(2298281);
                expec_nedges   = static_cast<tlo::perf::psample_val_t>(8906322);
                break;
            case 3:
                expec_nsamples = static_cast<tlo::perf::psample_val_t>(1471302);
                expec_nedges   = static_cast<tlo::perf::psample_val_t>(8906322);
                break;
            case 4:
            case 7:
                expec_nsamples =
                    2 * tlo::perf::k_func_scale_point *
                    std::max(k_confs[att % k_confs.size()].first, 1.0);
                expec_nedges =
                    2 * tlo::perf::k_edge_scale_point *
                    std::max(k_confs[att % k_confs.size()].second, 1.0);
                break;
            case 5:
            case 6:
                expec_nsamples =
                    tlo::perf::k_func_scale_point *
                    (std::max(k_confs[att % k_confs.size()].first, 1.0) + 1.0);
                expec_nedges =
                    tlo::perf::k_edge_scale_point *
                    (std::max(k_confs[att % k_confs.size()].second, 1.0) + 1.0);
                break;
                // NOLINTEND(*magic*)
            default:
                ASSERT_FALSE(true);
                break;
        }

        EXPECT_NEAR(check_func_stats.num_samples_, expec_nsamples, .99)
            << check_func_stats.num_samples_;
        EXPECT_EQ(check_func_stats.num_tracked_br_samples_in_,
                  static_cast<tlo::perf::psample_val_t>(8906322UL))
            << check_func_stats.num_tracked_br_samples_in_;
        EXPECT_EQ(check_func_stats.num_tracked_br_samples_out_,
                  static_cast<tlo::perf::psample_val_t>(8906322UL))
            << check_func_stats.num_tracked_br_samples_out_;
        EXPECT_EQ(check_func_stats.num_br_samples_in_,
                  static_cast<tlo::perf::psample_val_t>(48444158UL))
            << check_func_stats.num_br_samples_in_;
        EXPECT_EQ(check_func_stats.num_br_samples_out_,
                  static_cast<tlo::perf::psample_val_t>(48444158UL))
            << check_func_stats.num_br_samples_out_;
        EXPECT_NEAR(check_edge_stats.num_edges_, expec_nedges, .99)
            << check_edge_stats.num_edges_;

        close(fds_out[0]);
        close(fds_out[1]);
    }
}

TEST(perf, save_partial_states) {
    init_env();

    std::random_device rand_device;
    std::mt19937       rng(rand_device());
    for (const partial_state_info_t & save_state_info :
         k_combinable_save_states) {
        tlo::vec_t<std::string_view> ss_files = save_state_info.saved_states_;
        size_t                       i, j, e;
        i = 0;
        // NOLINTNEXTLINE(*magic*)
        e = std::min(static_cast<size_t>(4), ss_files.size());
        for (; i < e; ++i) {
            for (j = 0; j < 2; ++j) {
                tlo::sym::sym_state_t                  ss{};
                tlo::vec_t<tlo::perf::perf_func_t>     funcs;
                tlo::vec_t<tlo::perf::perf_edge_t>     edges;
                const tlo::perf::perf_state_reloader_t reloader{ &ss };
                tlo::perf::perf_state_scaling_t        scaling_todo{};
                if (j == 0) {
                    scaling_todo.set_force_no_scale();
                }
                ASSERT_TRUE(reloader.reload_state(&ss_files, &funcs, &edges,
                                                  &scaling_todo));
                if (save_state_info.unknown_) {
                    EXPECT_EQ(funcs.size(), 4217UL);
                    EXPECT_EQ(edges.size(), 7921UL);
                }
                else {
                    EXPECT_EQ(funcs.size(), 4093UL);
                    EXPECT_EQ(edges.size(), 2528UL);
                }

                tlo::perf::perf_func_stats_t check_func_stats{};
                tlo::perf::perf_edge_stats_t check_edge_stats{};
                uint64_t                     total_size = 0;
                for (const auto & pfunc : funcs) {
                    ASSERT_TRUE(pfunc.valid());
                    check_func_stats.add(pfunc.stats());
                    total_size += pfunc.func_clump_->size();
                }

                for (const auto & pedge : edges) {
                    ASSERT_TRUE(pedge.valid());
                    check_edge_stats.add(pedge.stats());
                    total_size += pedge.from_->size();
                    total_size += pedge.to_->size();
                }
                EXPECT_EQ(total_size, save_state_info.expec_total_size_)
                    << save_state_info.saved_states_[0];
                if (j == 0) {
                    if (save_state_info.unknown_) {
                        EXPECT_EQ(
                            check_func_stats.num_samples_,
                            static_cast<tlo::perf::psample_val_t>(65415UL));
                        EXPECT_EQ(
                            check_func_stats.num_tracked_br_samples_in_,
                            static_cast<tlo::perf::psample_val_t>(221055UL));
                        EXPECT_EQ(
                            check_func_stats.num_tracked_br_samples_out_,
                            static_cast<tlo::perf::psample_val_t>(221055UL));
                        EXPECT_EQ(
                            check_func_stats.num_br_samples_in_,
                            static_cast<tlo::perf::psample_val_t>(2326682UL));
                        EXPECT_EQ(
                            check_func_stats.num_br_samples_out_,
                            static_cast<tlo::perf::psample_val_t>(2326682UL));
                        EXPECT_EQ(
                            check_edge_stats.num_edges_,
                            static_cast<tlo::perf::psample_val_t>(221055UL));
                    }
                    else {
                        EXPECT_EQ(
                            check_func_stats.num_samples_,
                            static_cast<tlo::perf::psample_val_t>(15166UL));
                        EXPECT_EQ(
                            check_func_stats.num_tracked_br_samples_in_,
                            static_cast<tlo::perf::psample_val_t>(53428UL));
                        EXPECT_EQ(
                            check_func_stats.num_tracked_br_samples_out_,
                            static_cast<tlo::perf::psample_val_t>(53428UL));
                        EXPECT_EQ(
                            check_func_stats.num_br_samples_in_,
                            static_cast<tlo::perf::psample_val_t>(585577UL));
                        EXPECT_EQ(
                            check_func_stats.num_br_samples_out_,
                            static_cast<tlo::perf::psample_val_t>(573949UL));
                        EXPECT_EQ(
                            check_edge_stats.num_edges_,
                            static_cast<tlo::perf::psample_val_t>(53428L));
                    }
                }
                else {
                    if (save_state_info.unknown_) {
                        EXPECT_NEAR(check_func_stats.num_samples_,
                                    static_cast<tlo::perf::psample_val_t>(
                                        ss_files.size()) *
                                        tlo::perf::k_func_scale_point,
                                    .999);

                        EXPECT_EQ(
                            check_func_stats.num_tracked_br_samples_in_,
                            static_cast<tlo::perf::psample_val_t>(221055UL));
                        EXPECT_EQ(
                            check_func_stats.num_tracked_br_samples_out_,
                            static_cast<tlo::perf::psample_val_t>(221055UL));
                        EXPECT_EQ(
                            check_func_stats.num_br_samples_in_,
                            static_cast<tlo::perf::psample_val_t>(2326682UL));
                        EXPECT_EQ(
                            check_func_stats.num_br_samples_out_,
                            static_cast<tlo::perf::psample_val_t>(2326682UL));

                        EXPECT_NEAR(check_edge_stats.num_edges_,
                                    static_cast<tlo::perf::psample_val_t>(
                                        ss_files.size()) *
                                        tlo::perf::k_edge_scale_point,
                                    .999);
                    }
                    else {
                        size_t nfunc_expec = ss_files.size();
                        size_t nedge_expec = ss_files.size();
                        // NOLINTBEGIN(*magic*)
                        if (ss_files.size() == 884) {
                            // Because its split so much, there are some files
                            // where we have no edges/samples to scale.
                            nfunc_expec -= 7;
                            nedge_expec -= 13;
                        }
                        // NOLINTEND(*magic*)

                        EXPECT_NEAR(
                            check_func_stats.num_samples_,
                            static_cast<tlo::perf::psample_val_t>(nfunc_expec) *
                                tlo::perf::k_func_scale_point,
                            .999)
                            << ss_files.size();

                        EXPECT_EQ(
                            check_func_stats.num_tracked_br_samples_in_,
                            static_cast<tlo::perf::psample_val_t>(53428UL));
                        EXPECT_EQ(
                            check_func_stats.num_tracked_br_samples_out_,
                            static_cast<tlo::perf::psample_val_t>(53428UL));
                        EXPECT_EQ(
                            check_func_stats.num_br_samples_in_,
                            static_cast<tlo::perf::psample_val_t>(585577UL));
                        EXPECT_EQ(
                            check_func_stats.num_br_samples_out_,
                            static_cast<tlo::perf::psample_val_t>(573949UL));
                        EXPECT_NEAR(
                            check_edge_stats.num_edges_,
                            static_cast<tlo::perf::psample_val_t>(nedge_expec) *
                                tlo::perf::k_edge_scale_point,
                            .999);
                    }
                }
                // NOLINTBEGIN(*magic*)
                create_and_check_cg_order(&funcs, &edges,
                                          save_state_info.expec_order_path_,
                                          j == 0 ? 0.0 : 0.999);
                // NOLINTEND(*magic*)
                ASSERT_TRUE(G_okay) << ss_files[0];
            }
            std::shuffle(ss_files.begin(), ss_files.end(), rng);
        }
    }
}

TEST(perf, save_partial_states_cross_build) {
    init_env();
    std::random_device           rand_device;
    std::mt19937                 rng(rand_device());
    tlo::vec_t<std::string_view> ss_files = {
        INPUT_PATH "cross-build-id.0.saved",
        INPUT_PATH "cross-build-id.1.saved", INPUT_PATH "cross-build-id.2.saved"
    };
    size_t i, j, e;

    // NOLINTNEXTLINE(*magic*)
    e = std::min(static_cast<size_t>(3), ss_files.size());
    for (i = 0; i < e; ++i) {
        for (j = 0; j < 2; ++j) {
            tlo::sym::sym_state_t                  ss{};
            tlo::vec_t<tlo::perf::perf_func_t>     funcs;
            tlo::vec_t<tlo::perf::perf_edge_t>     edges;
            const tlo::perf::perf_state_reloader_t reloader{ &ss };
            tlo::perf::perf_state_scaling_t        scaling_todo{};
            if (j == 0) {
                scaling_todo.set_force_no_scale();
            }
            ASSERT_TRUE(reloader.reload_state(&ss_files, &funcs, &edges,
                                              &scaling_todo));

            EXPECT_EQ(funcs.size(), 22944UL);
            EXPECT_EQ(edges.size(), 48629UL);


            tlo::perf::perf_func_stats_t check_func_stats{};
            tlo::perf::perf_edge_stats_t check_edge_stats{};
            uint64_t                     total_size = 0;
            for (const auto & pfunc : funcs) {
                ASSERT_TRUE(pfunc.valid());
                check_func_stats.add(pfunc.stats());
                total_size += pfunc.func_clump_->size();
            }

            for (const auto & pedge : edges) {
                ASSERT_TRUE(pedge.valid());
                check_edge_stats.add(pedge.stats());
                total_size += pedge.from_->size();
                total_size += pedge.to_->size();
            }
            if (j == 0) {
                EXPECT_EQ(total_size, 139223582173UL);
                EXPECT_EQ(check_func_stats.num_samples_,
                          static_cast<tlo::perf::psample_val_t>(2984120UL));
                EXPECT_EQ(check_func_stats.num_tracked_br_samples_in_,
                          static_cast<tlo::perf::psample_val_t>(17612214UL));
                EXPECT_EQ(check_func_stats.num_tracked_br_samples_out_,
                          static_cast<tlo::perf::psample_val_t>(17612214UL));
                EXPECT_EQ(check_func_stats.num_br_samples_in_,
                          static_cast<tlo::perf::psample_val_t>(98252918UL));
                EXPECT_EQ(check_func_stats.num_br_samples_out_,
                          static_cast<tlo::perf::psample_val_t>(98252918UL));
                EXPECT_EQ(check_edge_stats.num_edges_,
                          static_cast<tlo::perf::psample_val_t>(17612214UL));
            }
            else {
                EXPECT_EQ(total_size, 139223582173UL);
                EXPECT_NEAR(check_func_stats.num_samples_,
                            3 * tlo::perf::k_func_scale_point, .999);
                EXPECT_EQ(check_func_stats.num_tracked_br_samples_in_,
                          static_cast<tlo::perf::psample_val_t>(17612214UL));
                EXPECT_EQ(check_func_stats.num_tracked_br_samples_out_,
                          static_cast<tlo::perf::psample_val_t>(17612214UL));
                EXPECT_EQ(check_func_stats.num_br_samples_in_,
                          static_cast<tlo::perf::psample_val_t>(98252918UL));
                EXPECT_EQ(check_func_stats.num_br_samples_out_,
                          static_cast<tlo::perf::psample_val_t>(98252918UL));
                EXPECT_NEAR(check_edge_stats.num_edges_,
                            3 * tlo::perf::k_edge_scale_point, .999);
            }
            // NOLINTBEGIN(*magic*)
            create_and_check_cg_order(&funcs, &edges,
                                      ORDER_PATH_FMT "-cross-build-id",
                                      j == 0 ? 0.0 : 0.1);
            // NOLINTEND(*magic*)
            ASSERT_TRUE(G_okay);
        }
        std::shuffle(ss_files.begin(), ss_files.end(), rng);
    }
}

