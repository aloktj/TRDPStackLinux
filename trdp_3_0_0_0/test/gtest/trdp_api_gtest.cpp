#include <array>
#include <cstdint>
#include <cstring>

#include <gtest/gtest.h>

#include "trdp_if_light.h"
#include "vos_sock.h"
#include "vos_thread.h"

namespace {

constexpr TRDP_IP_ADDR_T kLocalhost = 0x7f000001u;
constexpr UINT32 kMemorySize = 1024u * 1024u;
constexpr UINT32 kPdComId = 1000u;
constexpr UINT32 kPdReplyComId = 1001u;
constexpr UINT32 kPdCycle = 100000u;
constexpr UINT32 kPdTimeout = 500000u;
constexpr UINT32 kMdNotifyComId = 2000u;
constexpr UINT32 kMdRequestComId = 2001u;
constexpr UINT32 kMdReplyComId = 2002u;
constexpr UINT32 kReplyTimeout = 500000u;

void NoopLog(
    void *,
    TRDP_LOG_T,
    const CHAR8 *,
    const CHAR8 *,
    UINT16,
    const CHAR8 *)
{
}

struct MdListenerState {
    bool received_notify = false;
    bool received_request = false;
    TRDP_ERR_T reply_err = TRDP_NO_ERR;
    TRDP_URI_USER_T src_uri = "listener";
};

struct MdRequesterState {
    bool received_reply = false;
    TRDP_UUID_T reply_session_id{};
};

void MdListenerCallback(
    void *pRefCon,
    TRDP_APP_SESSION_T appHandle,
    const TRDP_MD_INFO_T *pMsg,
    UINT8 *,
    UINT32)
{
    auto *state = static_cast<MdListenerState *>(pRefCon);
    if (state == nullptr || pMsg == nullptr) {
        return;
    }

    if (pMsg->msgType == TRDP_MSG_MN) {
        state->received_notify = true;
        return;
    }

    if (pMsg->msgType == TRDP_MSG_MR) {
        const UINT8 reply_payload[] = {0xAB, 0xCD};
        state->received_request = true;
        state->reply_err = tlm_replyQuery(
            appHandle,
            &pMsg->sessionId,
            kMdReplyComId,
            0u,
            kReplyTimeout,
            nullptr,
            reply_payload,
            sizeof(reply_payload),
            state->src_uri);
    }
}

void MdRequesterCallback(
    void *pRefCon,
    TRDP_APP_SESSION_T,
    const TRDP_MD_INFO_T *pMsg,
    UINT8 *,
    UINT32)
{
    auto *state = static_cast<MdRequesterState *>(pRefCon);
    if (state == nullptr || pMsg == nullptr) {
        return;
    }

    if (pMsg->msgType == TRDP_MSG_MQ || pMsg->msgType == TRDP_MSG_MP) {
        state->received_reply = true;
        state->reply_session_id = pMsg->sessionId;
    }
}

void ProcessPdMd(TRDP_APP_SESSION_T appHandle, UINT32 iterations)
{
    for (UINT32 i = 0; i < iterations; ++i) {
        TRDP_TIME_T interval{};
        TRDP_FDS_T rfds;
        TRDP_SOCK_T no_desc = 0;
        VOS_FD_ZERO(&rfds);
        tlc_getInterval(appHandle, &interval, &rfds, &no_desc);
        if (no_desc > 0) {
            vos_select(no_desc, &rfds, nullptr, nullptr, &interval);
        }
        tlc_process(appHandle, &rfds, nullptr);
        vos_threadDelay(10u);
    }
}

}  // namespace

TEST(TrdpApiSmoke, ExercisesPublicApiOnLocalhost)
{
    std::array<UINT8, kMemorySize> memory{};
    TRDP_MEM_CONFIG_T memory_config{memory.data(), static_cast<UINT32>(memory.size()), {0u}};

    TRDP_PD_CONFIG_T pd_config{
        nullptr,
        nullptr,
        TRDP_PD_DEFAULT_SEND_PARAM,
        TRDP_FLAGS_NONE,
        TRDP_PD_DEFAULT_TIMEOUT,
        TRDP_TO_SET_TO_ZERO,
        TRDP_PD_UDP_PORT};

    TRDP_MD_CONFIG_T md_config{
        nullptr,
        nullptr,
        TRDP_MD_DEFAULT_SEND_PARAM,
        TRDP_FLAGS_NONE,
        TRDP_MD_DEFAULT_REPLY_TIMEOUT,
        TRDP_MD_DEFAULT_CONFIRM_TIMEOUT,
        TRDP_MD_DEFAULT_CONNECTION_TIMEOUT,
        TRDP_MD_DEFAULT_SENDING_TIMEOUT,
        TRDP_MD_UDP_PORT,
        TRDP_MD_TCP_PORT,
        4u};

    TRDP_PROCESS_CONFIG_T process_config{
        "gtest",
        "",
        "",
        TRDP_PROCESS_DEFAULT_CYCLE_TIME,
        TRDP_PROCESS_DEFAULT_PRIORITY,
        TRDP_PROCESS_DEFAULT_OPTIONS,
        0u};

    TRDP_APP_SESSION_T app_handle{};

    ASSERT_EQ(TRDP_NO_ERR, tlc_init(&NoopLog, nullptr, &memory_config));
    ASSERT_EQ(TRDP_NO_ERR,
              tlc_openSession(
                  &app_handle,
                  kLocalhost,
                  0u,
                  nullptr,
                  &pd_config,
                  &md_config,
                  &process_config));

    EXPECT_NE(nullptr, tlc_getVersionString());
    EXPECT_NE(nullptr, tlc_getVersion());

    EXPECT_EQ(TRDP_NO_ERR, tlc_reinitSession(app_handle));
    EXPECT_EQ(TRDP_NO_ERR, tlc_configSession(app_handle, nullptr, &pd_config, &md_config, &process_config));

    EXPECT_EQ(TRDP_NO_ERR, tlc_setETBTopoCount(app_handle, 1u));
    EXPECT_EQ(1u, tlc_getETBTopoCount(app_handle));
    EXPECT_EQ(TRDP_NO_ERR, tlc_setOpTrainTopoCount(app_handle, 2u));
    EXPECT_EQ(2u, tlc_getOpTrainTopoCount(app_handle));

    TRDP_TIME_T interval{};
    TRDP_FDS_T rfds;
    TRDP_SOCK_T no_desc = 0;
    VOS_FD_ZERO(&rfds);
    EXPECT_EQ(TRDP_NO_ERR, tlc_getInterval(app_handle, &interval, &rfds, &no_desc));
    EXPECT_EQ(TRDP_NO_ERR, tlc_process(app_handle, &rfds, nullptr));

    EXPECT_EQ(kLocalhost, tlc_getOwnIpAddress(app_handle));

    TRDP_PUB_T pub_handle{};
    TRDP_SUB_T sub_handle{};

    const UINT8 pd_payload[] = {0x11, 0x22, 0x33};
    ASSERT_EQ(TRDP_NO_ERR,
              tlp_subscribe(
                  app_handle,
                  &sub_handle,
                  nullptr,
                  nullptr,
                  0u,
                  kPdComId,
                  0u,
                  0u,
                  kLocalhost,
                  VOS_INADDR_ANY,
                  kLocalhost,
                  TRDP_FLAGS_NONE,
                  kPdTimeout,
                  TRDP_TO_SET_TO_ZERO));

    ASSERT_EQ(TRDP_NO_ERR,
              tlp_publish(
                  app_handle,
                  &pub_handle,
                  nullptr,
                  nullptr,
                  0u,
                  kPdComId,
                  0u,
                  0u,
                  kLocalhost,
                  kLocalhost,
                  kPdCycle,
                  0u,
                  TRDP_FLAGS_NONE,
                  pd_payload,
                  sizeof(pd_payload)));

    TRDP_IDX_TABLE_T index_config{};
    EXPECT_EQ(TRDP_NO_ERR, tlc_presetIndexSession(app_handle, &index_config));
    EXPECT_EQ(TRDP_NO_ERR, tlc_updateSession(app_handle));

    EXPECT_EQ(TRDP_NO_ERR, tlp_setRedundant(app_handle, 1u, TRUE));
    BOOL8 is_leader = FALSE;
    EXPECT_EQ(TRDP_NO_ERR, tlp_getRedundant(app_handle, 1u, &is_leader));

    EXPECT_EQ(TRDP_NO_ERR, tlp_put(app_handle, pub_handle, pd_payload, sizeof(pd_payload)));

    VOS_TIMEVAL_T tx_time{};
    EXPECT_EQ(TRDP_NO_ERR, tlp_putImmediate(app_handle, pub_handle, pd_payload, sizeof(pd_payload), &tx_time));
    EXPECT_EQ(TRDP_NO_ERR, tlp_processSend(app_handle));

    TRDP_TIME_T pd_interval{};
    TRDP_FDS_T pd_fds;
    TRDP_SOCK_T pd_no_desc = 0;
    VOS_FD_ZERO(&pd_fds);
    EXPECT_EQ(TRDP_NO_ERR, tlp_getInterval(app_handle, &pd_interval, &pd_fds, &pd_no_desc));
    if (pd_no_desc > 0) {
        vos_select(pd_no_desc, &pd_fds, nullptr, nullptr, &pd_interval);
    }
    EXPECT_EQ(TRDP_NO_ERR, tlp_processReceive(app_handle, &pd_fds, nullptr));

    EXPECT_EQ(TRDP_NO_ERR,
              tlp_request(
                  app_handle,
                  sub_handle,
                  0u,
                  kPdComId,
                  0u,
                  0u,
                  kLocalhost,
                  kLocalhost,
                  0u,
                  TRDP_FLAGS_NONE,
                  pd_payload,
                  sizeof(pd_payload),
                  kPdReplyComId,
                  kLocalhost));

    EXPECT_EQ(TRDP_NO_ERR, tlp_republish(app_handle, pub_handle, 0u, 0u, kLocalhost, kLocalhost));
    EXPECT_EQ(TRDP_NO_ERR, tlp_resubscribe(app_handle, sub_handle, 0u, 0u, kLocalhost, VOS_INADDR_ANY, kLocalhost));

    ProcessPdMd(app_handle, 5u);

    TRDP_PD_INFO_T pd_info{};
    std::array<UINT8, 16> pd_buffer{};
    UINT32 pd_size = static_cast<UINT32>(pd_buffer.size());
    TRDP_ERR_T pd_get_err = tlp_get(app_handle, sub_handle, &pd_info, pd_buffer.data(), &pd_size);
    EXPECT_EQ(TRDP_NO_ERR, pd_get_err);

    EXPECT_EQ(TRDP_NO_ERR, tlp_unpublish(app_handle, pub_handle));
    EXPECT_EQ(TRDP_NO_ERR, tlp_unsubscribe(app_handle, sub_handle));

#if MD_SUPPORT
    MdListenerState listener_state{};
    MdRequesterState requester_state{};
    TRDP_LIS_T listener_handle{};

    TRDP_URI_USER_T md_src_uri = "gtest-src";
    TRDP_URI_USER_T md_dest_uri = "gtest-dest";

    EXPECT_EQ(TRDP_NO_ERR,
              tlm_addListener(
                  app_handle,
                  &listener_handle,
                  &listener_state,
                  &MdListenerCallback,
                  TRUE,
                  kMdRequestComId,
                  0u,
                  0u,
                  kLocalhost,
                  VOS_INADDR_ANY,
                  kLocalhost,
                  TRDP_FLAGS_NONE,
                  md_src_uri,
                  md_dest_uri));

    EXPECT_EQ(TRDP_NO_ERR,
              tlm_readdListener(
                  app_handle,
                  listener_handle,
                  0u,
                  0u,
                  kLocalhost,
                  VOS_INADDR_ANY,
                  kLocalhost));

    EXPECT_EQ(TRDP_NO_ERR,
              tlm_notify(
                  app_handle,
                  nullptr,
                  nullptr,
                  kMdNotifyComId,
                  0u,
                  0u,
                  kLocalhost,
                  kLocalhost,
                  TRDP_FLAGS_NONE,
                  nullptr,
                  pd_payload,
                  sizeof(pd_payload),
                  md_src_uri,
                  md_dest_uri));

    for (int i = 0; i < 5 && !listener_state.received_notify; ++i) {
        TRDP_TIME_T md_interval{};
        TRDP_FDS_T md_fds;
        TRDP_SOCK_T md_no_desc = 0;
        VOS_FD_ZERO(&md_fds);
        EXPECT_EQ(TRDP_NO_ERR, tlm_getInterval(app_handle, &md_interval, &md_fds, &md_no_desc));
        if (md_no_desc > 0) {
            vos_select(md_no_desc, &md_fds, nullptr, nullptr, &md_interval);
        }
        EXPECT_EQ(TRDP_NO_ERR, tlm_process(app_handle, &md_fds, nullptr));
        vos_threadDelay(10u);
    }

    EXPECT_TRUE(listener_state.received_notify);

    TRDP_UUID_T request_session_id{};
    EXPECT_EQ(TRDP_NO_ERR,
              tlm_request(
                  app_handle,
                  &requester_state,
                  &MdRequesterCallback,
                  &request_session_id,
                  kMdRequestComId,
                  0u,
                  0u,
                  kLocalhost,
                  kLocalhost,
                  TRDP_FLAGS_NONE,
                  1u,
                  kReplyTimeout,
                  nullptr,
                  pd_payload,
                  sizeof(pd_payload),
                  md_src_uri,
                  md_dest_uri));

    for (int i = 0; i < 10 && !requester_state.received_reply; ++i) {
        TRDP_TIME_T md_interval{};
        TRDP_FDS_T md_fds;
        TRDP_SOCK_T md_no_desc = 0;
        VOS_FD_ZERO(&md_fds);
        EXPECT_EQ(TRDP_NO_ERR, tlm_getInterval(app_handle, &md_interval, &md_fds, &md_no_desc));
        if (md_no_desc > 0) {
            vos_select(md_no_desc, &md_fds, nullptr, nullptr, &md_interval);
        }
        EXPECT_EQ(TRDP_NO_ERR, tlm_process(app_handle, &md_fds, nullptr));
        vos_threadDelay(10u);
    }

    EXPECT_TRUE(listener_state.received_request);
    EXPECT_EQ(TRDP_NO_ERR, listener_state.reply_err);
    EXPECT_TRUE(requester_state.received_reply);

    EXPECT_EQ(TRDP_NO_ERR, tlm_confirm(app_handle, &requester_state.reply_session_id, 0u, nullptr));
    EXPECT_EQ(TRDP_NO_ERR, tlm_abortSession(app_handle, &request_session_id));

    EXPECT_EQ(TRDP_NO_ERR, tlm_delListener(app_handle, listener_handle));
#endif

    TRDP_STATISTICS_T statistics{};
    EXPECT_EQ(TRDP_NO_ERR, tlc_getStatistics(app_handle, &statistics));

    std::array<TRDP_SUBS_STATISTICS_T, 4> sub_stats{};
    UINT16 num_subs = static_cast<UINT16>(sub_stats.size());
    EXPECT_EQ(TRDP_NO_ERR, tlc_getSubsStatistics(app_handle, &num_subs, sub_stats.data()));

    std::array<TRDP_PUB_STATISTICS_T, 4> pub_stats{};
    UINT16 num_pubs = static_cast<UINT16>(pub_stats.size());
    EXPECT_EQ(TRDP_NO_ERR, tlc_getPubStatistics(app_handle, &num_pubs, pub_stats.data()));

#if MD_SUPPORT
    std::array<TRDP_LIST_STATISTICS_T, 4> udp_stats{};
    UINT16 num_udp = static_cast<UINT16>(udp_stats.size());
    EXPECT_EQ(TRDP_NO_ERR, tlc_getUdpListStatistics(app_handle, &num_udp, udp_stats.data()));

    std::array<TRDP_LIST_STATISTICS_T, 4> tcp_stats{};
    UINT16 num_tcp = static_cast<UINT16>(tcp_stats.size());
    EXPECT_EQ(TRDP_NO_ERR, tlc_getTcpListStatistics(app_handle, &num_tcp, tcp_stats.data()));
#endif

    std::array<TRDP_RED_STATISTICS_T, 4> red_stats{};
    UINT16 num_red = static_cast<UINT16>(red_stats.size());
    EXPECT_EQ(TRDP_NO_ERR, tlc_getRedStatistics(app_handle, &num_red, red_stats.data()));

    UINT16 num_join = 1u;
    UINT32 join_ip = 0u;
    EXPECT_EQ(TRDP_NO_ERR, tlc_getJoinStatistics(app_handle, &num_join, &join_ip));

    EXPECT_EQ(TRDP_NO_ERR, tlc_resetStatistics(app_handle));

    EXPECT_EQ(TRDP_NO_ERR, tlc_closeSession(app_handle));
    EXPECT_EQ(TRDP_NO_ERR, tlc_terminate());
}
