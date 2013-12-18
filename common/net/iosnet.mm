

#include "iosnet.h"

#ifdef _IOS
#include "net.h"

CFHostRef g_cfHost;
NSData* g_hostAddr;
CFSocketRef g_cfSocket;

NSString* ShowAddress(NSData* address)
// Returns a dotted decimal string for the specified address (a (struct sockaddr)
// within the address NSData).
{
    int         err;
    NSString *  result;
    char        hostStr[NI_MAXHOST];
    char        servStr[NI_MAXSERV];
    
    result = nil;
    
    if (address != nil)
    {
        
        // If it's a IPv4 address embedded in an IPv6 address, just bring it as an IPv4
        // address.  Remember, this is about display, not functionality, and users don't
        // want to see mapped addresses.
        
        if ([address length] >= sizeof(struct sockaddr_in6))
        {
            const struct sockaddr_in6 * addr6Ptr;
            
            addr6Ptr = (struct sockaddr_in6 *)[address bytes];
            if (addr6Ptr->sin6_family == AF_INET6)
            {
                if(IN6_IS_ADDR_V4MAPPED(&addr6Ptr->sin6_addr) || IN6_IS_ADDR_V4COMPAT(&addr6Ptr->sin6_addr))
                {
                    struct sockaddr_in  addr4;
                    
                    memset(&addr4, 0, sizeof(addr4));
                    addr4.sin_len         = sizeof(addr4);
                    addr4.sin_family      = AF_INET;
                    addr4.sin_port        = addr6Ptr->sin6_port;
                    addr4.sin_addr.s_addr = addr6Ptr->sin6_addr.__u6_addr.__u6_addr32[3];
                    address = [NSData dataWithBytes:&addr4 length:sizeof(addr4)];
                    assert(address != nil);
                }
            }
        }
        err = getnameinfo((struct sockaddr *)[address bytes], (socklen_t) [address length], hostStr, sizeof(hostStr), servStr, sizeof(servStr), NI_NUMERICHOST | NI_NUMERICSERV);
        if (err == 0)
        {
            result = [NSString stringWithFormat:@"%s:%s", hostStr, servStr];
            assert(result != nil);
        }
    }
    
    return result;
}

void NetError(NSError *error)
// Given an NSError, returns a short error string that we can print, handling
// some special cases along the way.
{
    NSString* result;
    NSNumber* failureNum;
    int failure;
    const char* failureStr;
    
    assert(error != nil);
    
    result = nil;
    
    // Handle DNS errors as a special case.
    
    if([[error domain] isEqual:(NSString *)kCFErrorDomainCFNetwork] && ([error code] == kCFHostErrorUnknown))
    {
        failureNum = [[error userInfo] objectForKey:(id)kCFGetAddrInfoFailureKey];
        if ( [failureNum isKindOfClass:[NSNumber class]] )
        {
            failure = [failureNum intValue];
            if (failure != 0)
            {
                failureStr = gai_strerror(failure);
                if (failureStr != NULL)
                {
                    result = [NSString stringWithUTF8String:failureStr];
                    assert(result != nil);
                }
            }
        }
    }
    
    // Otherwise try various properties of the error object.
    
    if (result == nil)
        result = [error localizedFailureReason];
    
    if (result == nil)
        result = [error localizedDescription];
    
    if (result == nil)
        result = [error description];
    
    assert(result != nil);
    
    NSLog(@"Net error: %@", result);
    Error([result UTF8String]);
    ClearPackets();
}

void NetStreamError(CFStreamError streamError)
// Stops the object, reporting the supplied error to the delegate.
{
    NSDictionary* userInfo;
    NSError* error;
    
    if (streamError.domain == kCFStreamErrorDomainNetDB)
        userInfo = [NSDictionary dictionaryWithObjectsAndKeys: [NSNumber numberWithInteger:streamError.error], kCFGetAddrInfoFailureKey, nil];
    else
        userInfo = nil;
        
    error = [NSError errorWithDomain:(NSString *)kCFErrorDomainCFNetwork code:kCFHostErrorUnknown userInfo:userInfo];
    
    NetError(error);
}

static void SocketCallback(CFSocketRef s, CFSocketCallBackType type, CFDataRef address, const void *data, void *info)
// This C routine is called by CFSocket when there's data waiting on our
// UDP socket.  It just redirects the call to Objective-C code.
{
    if(type == kCFSocketReadCallBack)
        ReadPackets();
}

void StopHostResolution()
// Called to stop the CFHost part of the object, if it's still running.
{
    if (g_cfHost == NULL)
        return;
    
    CFHostSetClient(g_cfHost, NULL, NULL);
    CFHostCancelInfoResolution(g_cfHost, kCFHostAddresses);
    CFHostUnscheduleFromRunLoop(g_cfHost, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
    CFRelease(g_cfHost);
    g_cfHost = NULL;
}

// Called by our CFHost resolution callback (HostResolveCallback) when host
// resolution is complete.  We find the best IP address and create a socket
// connected to that.
static void HostResolveCallback(CFHostRef theHost, CFHostInfoType typeInfo, const CFStreamError *error, void *info)
// This C routine is called by CFHost when the host resolution is complete.
// It just redirects the call to the appropriate Objective-C method.
{
    g_cfHost = theHost;
    
    if(typeInfo != kCFHostAddresses)
        return;
    
    if((error != NULL) && (error->domain != 0))
    {
        NetStreamError(*error);
        return;
    }
    
    NSError* nserror = nil;
    Boolean resolved;
    NSArray* resolvedAddresses;
    
    // Walk through the resolved addresses looking for one that we can work with.
    
    resolvedAddresses = (__bridge NSArray *) CFHostGetAddressing(g_cfHost, &resolved);
    
    if(resolved && (resolvedAddresses != nil))
    {
        for(NSData* address in resolvedAddresses)
        {
            bool success;
            const struct sockaddr* addrPtr;
            NSUInteger addrLen;
            
            addrPtr = (const struct sockaddr *)[address bytes];
            addrLen = [address length];
            //assert(addrLen >= sizeof(struct sockaddr));
            
            // Try to create a connected CFSocket for this address.  If that fails,
            // we move along to the next address.  If it succeeds, we're done.
            
            success = NO;
            if(addrPtr->sa_family != AF_INET
#ifndef IPV4_ONLY
                && addrPtr->sa_family != AF_INET6
#endif
                )
                continue;
            
            success = Connect(address, PORT, &nserror);
                
            if(!success)
                continue;
                
            CFDataRef hostAddress;
                    
            hostAddress = CFSocketCopyPeerAddress(g_cfSocket);
            assert(hostAddress != NULL);
            
            g_hostAddr = (__bridge NSData *)hostAddress;
                    
            CFRelease(hostAddress);
            
            memcpy(&g_sockaddr, addrPtr, sizeof(struct sockaddr));
            //memcpy(&g_sockaddr, [g_hostAddr bytes], sizeof(struct sockaddr));
            //g_sockaddr = *addrPtr;
            
            NSLog(@"Connected to %@", ShowAddress(g_hostAddr));
                
            break;
        }
    }
    
    // If we didn't get an address and didn't get an error, synthesise a host not found error.
    
    if((g_hostAddr == nil) && (nserror == nil))
        nserror = [NSError errorWithDomain:(NSString *)kCFErrorDomainCFNetwork code:kCFHostErrorHostNotFound userInfo:nil];
    
    if(nserror == nil)
    {
        // We're done resolving, so shut that down.
        
        StopHostResolution();
        
        NSLog(@"Resolved host %@", SERVER_ADDR);
    }
    else
        NetError(nserror);
}

bool Connect(NSData* address, NSUInteger port, NSError** errorPtr)
// Sets up the CFSocket in either client or server mode.  In client mode,
// address contains the address that the socket should be connected to.
// The address contains zero port number, so the port parameter is used instead.
// In server mode, address is nil and the socket is bound to the wildcard
// address on the specified port.
{
    sa_family_t socketFamily;
    int err;
    int junk;
    int sock;
    CFRunLoopSourceRef rls;
    
    // Create the UDP socket itself.  First try IPv6 and, if that's not available, revert to IPv6.
    //
    // IMPORTANT: Even though we're using IPv6 by default, we can still work with IPv4 due to the
    // miracle of IPv4-mapped addresses.
    
    err = 0;
    sock = socket(AF_INET6, SOCK_DGRAM, 0);
    if (sock >= 0)
        socketFamily = AF_INET6;
    else
    {
        sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock >= 0)
            socketFamily = AF_INET;
        else
        {
            err = errno;
            socketFamily = 0;       // quietens a warning from the compiler
            assert(err != 0);       // Obvious, but it quietens a warning from the static analyser.
        }
    }
    
    // Bind or connect the socket, depending on whether we're in server or client mode.
    
    if(err == 0)
    {
        struct sockaddr_storage addr;
        struct sockaddr_in *    addr4;
        struct sockaddr_in6 *   addr6;
        
        addr4 = (struct sockaddr_in * ) &addr;
        addr6 = (struct sockaddr_in6 *) &addr;
        
        memset(&addr, 0, sizeof(addr));
        
        // Client mode.  Set up the address on the caller-supplied address and port
        // number.  Also, if the address is IPv4 and we created an IPv6 socket,
        // convert the address to an IPv4-mapped address.
        if([address length] > sizeof(addr))
        {
            assert(NO);         // very weird
            [address getBytes:&addr length:sizeof(addr)];
        }
        else
            [address getBytes:&addr length:[address length]];
        
        if(addr.ss_family == AF_INET)
        {
            if(socketFamily == AF_INET6)
            {
                struct in_addr ipv4Addr;
                    
                // Convert IPv4 address to IPv4-mapped-into-IPv6 address.
                    
                ipv4Addr = addr4->sin_addr;
                    
                addr6->sin6_len = sizeof(*addr6);
                addr6->sin6_family = AF_INET6;
                addr6->sin6_port = htons(port);
                addr6->sin6_addr.__u6_addr.__u6_addr32[0] = 0;
                addr6->sin6_addr.__u6_addr.__u6_addr32[1] = 0;
                addr6->sin6_addr.__u6_addr.__u6_addr16[4] = 0;
                addr6->sin6_addr.__u6_addr.__u6_addr16[5] = 0xffff;
                addr6->sin6_addr.__u6_addr.__u6_addr32[3] = ipv4Addr.s_addr;
            }
            else
                addr4->sin_port = htons(port);
        }
        else
        {
            assert(addr.ss_family == AF_INET6);
            addr6->sin6_port        = htons(port);
        }
        if( (addr.ss_family == AF_INET) && (socketFamily == AF_INET6) )
        {
            addr6->sin6_len         = sizeof(*addr6);
            addr6->sin6_port        = htons(port);
            addr6->sin6_addr        = in6addr_any;
        }
        
        err = connect(sock, (const struct sockaddr *) &addr, addr.ss_len);
        
        if(err < 0)
            err = errno;
    }
    
    // From now on we want the socket in non-blocking mode to prevent any unexpected
    // blocking of the main thread.  None of the above should block for any meaningful
    // amount of time.
    
    if(err == 0)
    {
        int flags;
        
        flags = fcntl(sock, F_GETFL);
        err = fcntl(sock, F_SETFL, flags | O_NONBLOCK);
        
        if(err < 0)
            err = errno;
    }
    
    // Wrap the socket in a CFSocket that's scheduled on the runloop.
    
    if(err == 0)
    {
        g_cfSocket = CFSocketCreateWithNative(NULL, sock, kCFSocketReadCallBack, SocketCallback, NULL);
        
        // The socket will now take care of cleaning up our file descriptor.
        
        assert( CFSocketGetSocketFlags(g_cfSocket) & kCFSocketCloseOnInvalidate );
        sock = -1;
        
        rls = CFSocketCreateRunLoopSource(NULL, g_cfSocket, 0);
        assert(rls != NULL);
        
        CFRunLoopAddSource(CFRunLoopGetCurrent(), rls, kCFRunLoopDefaultMode);
        
        CFRelease(rls);
    }
    
    // Handle any errors.
    
    if(sock != -1)
        junk = close(sock);
        //assert(junk == 0);
    
    //assert( (err == 0) == (g_cfSocket != NULL) );
    
    if( (g_cfSocket == NULL) && (errorPtr != NULL) )
        *errorPtr = [NSError errorWithDomain:NSPOSIXErrorDomain code:err userInfo:nil];
    
    return (err == 0);
}

void ResolveHost(NSString* hostName, NSUInteger port)
{
    //assert( (port > 0) && (port < 65536) );
    
    bool success;
    CFHostClientContext context = {0, NULL, NULL, NULL, NULL};
    CFStreamError streamError;
        
    g_cfHost = CFHostCreateWithName(NULL, (__bridge CFStringRef) hostName);
        
    CFHostSetClient(g_cfHost, HostResolveCallback, &context);
        
    CFHostScheduleWithRunLoop(g_cfHost, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
        
    success = CFHostStartInfoResolution(g_cfHost, kCFHostAddresses, &streamError);
    
    if(!success)
        NetStreamError(streamError);
}

void InitNetIOS()
{
	NSString* nsstrhost = [NSString stringWithCString:SV_ADDR encoding:NSASCIIStringEncoding];
    ResolveHost(nsstrhost, PORT);
}

#endif