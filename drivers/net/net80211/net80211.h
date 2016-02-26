/*++

Copyright (c) 2015 Minoca Corp. All Rights Reserved

Module Name:

    net80211.h

Abstract:

    This header contains definitions for the 802.11 core wireless networking
    library.

Author:

    Chris Stevens 19-Oct-2015

--*/

//
// ------------------------------------------------------------------- Includes
//

#include <minoca/kernel/driver.h>
#include <minoca/net/netdrv.h>

//
// Redefine the API define into an export.
//

#define NET80211_API DLLEXPORT

#include <minoca/net/net80211.h>
#include <minoca/net/net8022.h>

//
// ---------------------------------------------------------------- Definitions
//

#define NET80211_ALLOCATION_TAG 0x69666957 // 'ifiW'

//
// Define the maximum number of supported keys.
//

#define NET80211_MAX_KEY_COUNT NET80211_CCMP_MAX_KEY_COUNT

//
// Define the maximum passphrase supported by 802.11 encryption. WPA2-PSK has
// a maximum of 64 characters. This is internal as it may change over time.
//

#define NET80211_MAX_PASSPHRASE_LENGTH 64

//
// Define the set of scan flags.
//

#define NET80211_SCAN_FLAG_BACKGROUND 0x00000001
#define NET80211_SCAN_FLAG_BROADCAST  0x00000002
#define NET80211_SCAN_FLAG_JOIN       0x00000004

//
// Define the default amount of time to wait on a channel during a scan.
//

#define NET80211_DEFAULT_SCAN_DWELL_TIME (100 * MICROSECONDS_PER_MILLISECOND)

//
// Define the set of 802.11 link flags.
//

#define NET80211_LINK_FLAG_DATA_PAUSED  0x00000001
#define NET80211_LINK_FLAG_TIMER_QUEUED 0x00000002

//
// Define the set of BSS entry flags.
//

#define NET80211_BSS_FLAG_ENCRYPT_DATA 0x00000001

//
// ------------------------------------------------------ Data Type Definitions
//

typedef enum _NET80211_ENCRYPTION_TYPE {
    Net80211EncryptionNone,
    Net80211EncryptionWep,
    Net80211EncryptionWpaPsk,
    Net80211EncryptionWpa2Psk
} NET80211_ENCRYPTION_TYPE, *PNET80211_ENCRYPTION_TYPE;

/*++

Structure Description:

    This structure defines the scan state for the 802.11 node.

Members:

    Link - Stores a pointer to the 802.11 link over which the scan will be
        performed.

    Flags - Stores a bitmask of flags that dictate the scan's behavior. See
        NET80211_SCAN_FLAG_* for definitions.

    Channel - Stores the current channel that is being scanned.

    Bssid - Stores an optional BSSID to be used when scanning for a specific
        BSS. This is only valid when the broadcast flag is not set.

    SsidLength - Stores the number of characters in the SSID for which the scan
        is searching.

    Ssid - Stores the SSID of the ESS for which the scan is searching.

    PassphraseLength - Stores the number of characters in the passphrase of the
        ESS for which the scan is searching.

    Passphrase - Stores the passphrase of the ESS for which the scan is
        searching.

--*/

typedef struct _NET80211_SCAN_STATE {
    PNET80211_LINK Link;
    ULONG Flags;
    ULONG Channel;
    UCHAR Bssid[NET80211_ADDRESS_SIZE];
    ULONG SsidLength;
    UCHAR Ssid[NET80211_MAX_SSID_LENGTH];
    ULONG PassphraseLength;
    UCHAR Passphrase[NET80211_MAX_PASSPHRASE_LENGTH];
} NET80211_SCAN_STATE, *PNET80211_SCAN_STATE;

/*++

Structure Description:

    This structure defines a key for the 802.11 core networking library.

Members:

    Flags - Stores a bitmask of flags describing the key. See
        NET80211_KEY_FLAG_* for definitions.

    Id - Stores the key ID negotiatied between the station and access point.

    Length - Stores the length of the key, in bytes.

    PacketNumber - Stores a 64-bit packet number that is incremented for each
        encrypted packet sent with the key. Only the lower 48-bits are used.

    ReplayCounter - Stores a 64-bit replay counter that is updated on each
        accepted data frame. All subsequent data frames must have a packet
        number greater than the current replay counter.

    Value - Stores the array holding the value of the key.

--*/

typedef struct _NET80211_KEY {
    ULONG Flags;
    ULONG Id;
    ULONG Length;
    volatile ULONGLONG PacketNumber;
    ULONGLONG ReplayCounter;
    UCHAR Value[ANYSIZE_ARRAY];
} NET80211_KEY, *PNET80211_KEY;

/*++

Structure Description:

    This structure defines the encryption information for a BSS.

Members:

    Pairwise - Stores the pairwise encryption algorithm.

    Group - Stores the group encryption algorithm.

    Keys - Stores an array of keys for an authenticated connection.

    ApRsn - Stores a pointer to the RSN information gathered from the BSS's AP.

    StationRsn - Stores a pointer to the local RSN information for this station.

--*/

typedef struct _NET80211_ENCRYPTION {
    NET80211_ENCRYPTION_TYPE Pairwise;
    NET80211_ENCRYPTION_TYPE Group;
    PNET80211_KEY Keys[NET80211_MAX_KEY_COUNT];
    PUCHAR ApRsn;
    PUCHAR StationRsn;
} NET80211_ENCRYPTION, *PNET80211_ENCRYPTION;

/*++

Structure Description:

    This structure defines an available BSS that is within range of the 802.11
    wireless device.

Members:

    ListEntry - Stores pointers to the next and previous BSS entries.

    ReferenceCount - Stores the reference count on the BSS entry.

    State - Stores the BSS state.

    Encryption - Stores the encryption information for the BSS including the
        active keys.

    EapolHandle - Stores a pointer to the encryption handshake handle.

    Flags - Stores a bitmask of flags describing the BSS entry. See
        NET80211_BSS_FLAG_* for definitions.

    SsidLength - Stores the number of characters in the SSID.

    Ssid - Stores the SSID for the BSS.

    PassphraseLength - Stores the number of characters in the passphrase.

    Passphrase - Stores the passphrase for the BSS.

--*/

typedef struct _NET80211_BSS_ENTRY {
    LIST_ENTRY ListEntry;
    volatile ULONG ReferenceCount;
    NET80211_BSS State;
    NET80211_ENCRYPTION Encryption;
    HANDLE EapolHandle;
    ULONG Flags;
    ULONG SsidLength;
    UCHAR Ssid[NET80211_MAX_SSID_LENGTH];
    ULONG PassphraseLength;
    UCHAR Passphrase[NET80211_MAX_PASSPHRASE_LENGTH];
} NET80211_BSS_ENTRY, *PNET80211_BSS_ENTRY;

/*++

Structure Description:

    This structure defines link information that is private to the 802.11 core.

Members:

    NetworkLink - Stores a pointer to the networking core's link context.

    ReferenceCount - Stores the reference count of the link.

    State - Stores the current state of the 802.11 link.

    Flags - Stores a bitmask of flags describing the link.
        See NET80211_LINK_FLAG_* for definitions.

    SequenceNumber - Stores the current sequence for the 802.11 link.

    Lock - Stores a pointer to a queued lock that synchronizes access to the
        802.11 link structure.

    StateTimer - Stores a pointer to a timer used to monitor state transitions.

    TimeoutDpc- Stores a pointer to a DPC that is executed when the state timer
        expires.

    TimeoutWorkItem - Stores a pointer to a low level work item queued by the
        DPC on state transition timeouts.

    BssList - Stores a list of BSSs that are within range of this 802.11 device.

    ActiveBss - Stores a pointer to the BSS to which the link is attempting to
        connect or is already connected.

    PausedPacketList - Stores a list of paused 802.11 packets that are ready
        for transmission.

    Properites - Stores the 802.11 link properties.

--*/

struct _NET80211_LINK {
    PNET_LINK NetworkLink;
    volatile ULONG ReferenceCount;
    NET80211_STATE State;
    ULONG Flags;
    volatile ULONG SequenceNumber;
    PQUEUED_LOCK Lock;
    PKTIMER StateTimer;
    PDPC TimeoutDpc;
    PWORK_ITEM TimeoutWorkItem;
    LIST_ENTRY BssList;
    PNET80211_BSS_ENTRY ActiveBss;
    NET_PACKET_LIST PausedPacketList;
    NET80211_LINK_PROPERTIES Properties;
};

//
// -------------------------------------------------------------------- Globals
//

//
// -------------------------------------------------------- Function Prototypes
//

VOID
Net80211pProcessManagementFrame (
    PNET80211_LINK Link,
    PNET_PACKET_BUFFER Packet
    );

/*++

Routine Description:

    This routine processes 802.11 management frames.

Arguments:

    Link - Supplies a pointer to the 802.11 link on which the frame arrived.

    Packet - Supplies a pointer to the network packet.

Return Value:

    None.

--*/

KSTATUS
Net80211pStartScan (
    PNET80211_LINK Link,
    PNET80211_SCAN_STATE Parameters
    );

/*++

Routine Description:

    This routine starts a scan for one or more BSSs within range of this
    station.

Arguments:

    Link - Supplies a pointer to the link on which to perform the scan.

    Parameters - Supplies a pointer to a scan state used to initialize the
        scan. This memory will not be referenced after the function returns,
        so this may be a stack allocated structure.

Return Value:

    Status code.

--*/

VOID
Net80211pSetState (
    PNET80211_LINK Link,
    NET80211_STATE State
    );

/*++

Routine Description:

    This routine sets the given link's 802.11 state by alerting the driver of
    the state change and then performing any necessary actions based on the
    state transition.

Arguments:

    Link - Supplies a pointer to the 802.11 link whose state is being updated.

    State - Supplies the state to which the link is transitioning.

Return Value:

    None.

--*/

PNET80211_BSS_ENTRY
Net80211pGetBss (
    PNET80211_LINK Link
    );

/*++

Routine Description:

    This routine gets the link's active BSS entry and hands back a pointer with
    a reference to the caller.

Arguments:

    Link - Supplies a pointer to the 802.11 link whose active BSS is to be
        returned.

Return Value:

    Returns a pointer to the active BSS.

--*/

VOID
Net80211pBssEntryAddReference (
    PNET80211_BSS_ENTRY BssEntry
    );

/*++

Routine Description:

    This routine increments the reference count of the given BSS entry.

Arguments:

    BssEntry - Supplies a pointer to the BSS entry whose reference count is to
        be incremented.

Return Value:

    None.

--*/

VOID
Net80211pBssEntryReleaseReference (
    PNET80211_BSS_ENTRY BssEntry
    );

/*++

Routine Description:

    This routine decrements the reference count of the given BSS entry,
    destroying the entry if there are no more references.

Arguments:

    BssEntry - Supplies a pointer to the BSS entry whose reference count is to
        be decremented.

Return Value:

    None.

--*/

VOID
Net80211pStateTimeoutDpcRoutine (
    PDPC Dpc
    );

/*++

Routine Description:

    This routine implements the 802.11 state transition timeout DPC that gets
    called after a remote node does not respond to a management frame.

Arguments:

    Dpc - Supplies a pointer to the DPC that is running.

Return Value:

    None.

--*/

VOID
Net80211pStateTimeoutWorker (
    PVOID Parameter
    );

/*++

Routine Description:

    This routine performs the low level work when an 802.11 state transition
    times out due to a remote node not responding.

Arguments:

    Parameter - Supplies a pointer to the nework link whose 802.11 state
        transition has timed out.

Return Value:

    None.

--*/

VOID
Net80211pProcessControlFrame (
    PNET80211_LINK Link,
    PNET_PACKET_BUFFER Packet
    );

/*++

Routine Description:

    This routine processes an 802.11 control frame.

Arguments:

    Link - Supplies a pointer to the 802.11 link on which the frame arrived.

    Packet - Supplies a pointer to the network packet.

Return Value:

    None.

--*/

KSTATUS
Net80211pSendDataFrames (
    PNET80211_LINK Link,
    PNET_PACKET_LIST PacketList,
    PNETWORK_ADDRESS SourcePhysicalAddress,
    PNETWORK_ADDRESS DestinationPhysicalAddress,
    ULONG ProtocolNumber
    );

/*++

Routine Description:

    This routine adds 802.2 SAP headers and 802.11 data frame headers to the
    given packets and sends them down the the device link layer.

Arguments:

    Link - Supplies a pointer to the 802.11 link on which to send the data.

    PacketList - Supplies a pointer to a list of network packets to send. Data
        in these packets may be modified by this routine, but must not be used
        once this routine returns.

    SourcePhysicalAddress - Supplies a pointer to the source (local) physical
        network address.

    DestinationPhysicalAddress - Supplies the optional physical address of the
        destination, or at least the next hop. If NULL is provided, then the
        packets will be sent to the data link layer's broadcast address.

    ProtocolNumber - Supplies the protocol number of the data inside the data
        link header.

Return Value:

    Status code.

--*/

VOID
Net80211pProcessDataFrame (
    PNET80211_LINK Link,
    PNET_PACKET_BUFFER Packet
    );

/*++

Routine Description:

    This routine processes an 802.11 data frame.

Arguments:

    Link - Supplies a pointer to the 802.11 link on which the frame arrived.

    Packet - Supplies a pointer to the network packet.

Return Value:

    None.

--*/

VOID
Net80211pPauseDataFrames (
    PNET80211_LINK Link
    );

/*++

Routine Description:

    This routine pauses the outgoing data frame traffic on the given network
    link. It assumes that the 802.11 link's queued lock is held.

Arguments:

    Link - Supplies a pointer to the 802.11 link on which to pause the outgoing
        data frames.

Return Value:

    None.

--*/

VOID
Net80211pResumeDataFrames (
    PNET80211_LINK Link
    );

/*++

Routine Description:

    This routine resumes the outgoing data frame traffic on the given network
    link, flushing any packets that were held while the link was paused. It
    assumes that the 802.11 link's queued lock is held.

Arguments:

    Link - Supplies a pointer to the 802.11 link on which to resume the
        outgoing data frames.

Return Value:

    None.

--*/

ULONG
Net80211pGetSequenceNumber (
    PNET80211_LINK Link
    );

/*++

Routine Description:

    This routine returns the sequence number to use for the given link.

Arguments:

    Link - Supplies a pointer to the 802.11 link whose sequence number is
        requested.

Return Value:

    Returns the sequence number to use for the given link.

--*/

KSTATUS
Net80211pSetChannel (
    PNET80211_LINK Link,
    ULONG Channel
    );

/*++

Routine Description:

    This routine sets the 802.11 link's channel to the given value.

Arguments:

    Link - Supplies a pointer to the 802.11 link whose channel is being updated.

    Channel - Supplies the channel to which the link should be set.

Return Value:

    Status code.

--*/

KSTATUS
Net80211pInitializeEncryption (
    PNET80211_LINK Link,
    PNET80211_BSS_ENTRY Bss
    );

/*++

Routine Description:

    This routine initializes the 802.11 core to handle the completion of an
    advanced encryption handshake.

Arguments:

    Link - Supplies a pointer to the 802.11 link establishing an ecrypted
        connection.

    Bss - Supplies a pointer to the BSS on which the encryption handshake will
        take place.

Return Value:

    Status code.

--*/

VOID
Net80211pDestroyEncryption (
    PNET80211_BSS_ENTRY Bss
    );

/*++

Routine Description:

    This routine destroys the context used to handle encryption initialization.
    It is not necessary to keep this context once the encrypted state is
    reached.

Arguments:

    Bss - Supplies a pointer to the BSS on which encryption initialization took
        place.

Return Value:

    None.

--*/

KSTATUS
Net80211pEncryptPacket (
    PNET80211_LINK Link,
    PNET80211_BSS_ENTRY Bss,
    PNET_PACKET_BUFFER Packet
    );

/*++

Routine Description:

    This routine encrypts the given network packet's plaintext data. The
    supplied packet buffer is modified directly and should already include the
    full MPDU (i.e. the 802.11 headers should be present).

Arguments:

    Link - Supplies a pointer to the 802.11 network link that owns the packet.

    Bss - Supplies a pointer to the BSS over which this packet should be sent.

    Packet - Supplies a pointer to the packet to encrypt.

Return Value:

    Status code.

--*/

KSTATUS
Net80211pDecryptPacket (
    PNET80211_LINK Link,
    PNET80211_BSS_ENTRY Bss,
    PNET_PACKET_BUFFER Packet
    );

/*++

Routine Description:

    This routine decrypts the given network packet's ciphertext. The supplied
    packet buffer is modified directly and should contain the full encrypted
    MPDU, including the 802.11 headers.

Arguments:

    Link - Supplies a pointer to the 802.11 network link that owns the packet.

    Bss - Supplies a pointer to the BSS over which this packet was received.

    Packet - Supplies a pointer to the packet to decrypt.

Return Value:

    Status code.

--*/

KSTATUS
Net80211pEapolInitialize (
    VOID
    );

/*++

Routine Description:

    This routine initializes support for EAPOL packets.

Arguments:

    None.

Return Value:

    Status code.

--*/

VOID
Net80211pEapolDestroy (
    VOID
    );

/*++

Routine Description:

    This routine tears down support for EAPOL packets.

Arguments:

    None.

Return Value:

    None.

--*/
