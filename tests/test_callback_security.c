#include "../src/qq/callback_security.h"
#include <assert.h>
#include <stdio.h>
int main(void) {
    qq_callback_target target;
    assert(qq_callback_ipv4_is_public("8.8.8.8"));
    assert(!qq_callback_ipv4_is_public("127.0.0.1"));
    assert(!qq_callback_ipv4_is_public("10.1.2.3"));
    assert(!qq_callback_ipv4_is_public("169.254.169.254"));
    assert(!qq_callback_ipv4_is_public("192.168.1.1"));
    assert(!qq_callback_ipv4_is_public("100.64.0.1"));
    assert(!qq_callback_ipv4_is_public("224.0.0.1"));
    assert(qq_callback_ipv6_is_public("2606:4700:4700::1111"));
    assert(!qq_callback_ipv6_is_public("::1"));
    assert(!qq_callback_ipv6_is_public("fc00::1"));
    assert(!qq_callback_ipv6_is_public("fe80::1"));
    assert(!qq_callback_ipv6_is_public("2001:db8::1"));
    assert(!qq_callback_ipv6_is_public("::ffff:127.0.0.1"));
    assert(qq_callback_target_validate("http://example.com/callback", &target) != 0);
    assert(qq_callback_target_validate("https://127.0.0.1/callback", &target) != 0);
    assert(qq_callback_target_validate("https://user:pass@example.com/hook", &target) != 0);
    assert(qq_callback_target_validate("https://example.com:8443/hook", &target) != 0);
    puts("callback security tests passed");
    return 0;
}
