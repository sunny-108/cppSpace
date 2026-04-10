# Jira Tickets Summary — Sunny Shivam (sshivam@hpe.com)

> **Note:** Jira instance (jira-pro.its.hpecorp.net) was unreachable from this network.
> Tickets below are extracted from GitHub PR titles referencing Jira IDs (STO-*, Bug *, AT-*).

---

## Jira Tickets by Feature Area

### Dual Authorization Framework (2026)
| Ticket | Title | Repos Affected |
|--------|-------|----------------|
| STO-7652 | Event log entries missing for DA requests on Catalyst item delete | so-pml-core |
| STO-7111 | DA generates duplicate deletion requests for bidirectional NAS share mappings | so-pml-core |
| STO-7085 | Protect Delete Catalyst Item with Dual Auth — Test Automation | StoreOnce-Test |
| STO-7084 | No events in Event Log for DA requests on NAS rep mappings | so-pml-core |
| STO-6977 | Localize Catalyst Item Dual Auth Strings | pml-storeonce-platform-language-pack |
| STO-6976 | Register Catalyst Item Delete in Dual Auth Framework | so-pml-core |
| STO-6975 | Dual Auth Validation Hook for Catalyst Item Delete | pml-storeonce-cat |
| STO-6727 | Dual Auth for NAS — Test Automation | StoreOnce-Test |
| STO-6726 | Localization — Dual Auth for NAS | pml-storeonce-platform-language-pack |
| STO-6694 | Enable Dual Auth Support for NAS Share Operations | so-pml-core |
| STO-6693 | Add Dual Auth Validation to NAS REST Endpoints | pml-storeonce-nas |
| STO-6539 | DA for NAS and VTL Replication Mapping — Localization | pml-storeonce-platform-language-pack |
| STO-6391 | Dual Authorization Replication Mapping — Test Automation | StoreOnce-Test |
| STO-6282 | Add Dual Auth Validation to Replication REST Endpoints | pml-storeonce-rep |
| STO-6269 | Implement Dual Auth Infrastructure for NAS and VTL Replication Mappings | so-pml-core |

### Code Coverage & Quality Initiative (2025)
| Ticket | Title | Repos Affected |
|--------|-------|----------------|
| STO-4163 | README File for Catalyst Plugin | so-d2d-catalyst_plugins |
| STO-4102 | Baseline Code Coverage — pml-storeonce-fc | pml-storeonce-fc |
| STO-4101 | Baseline Code Coverage — pml-storeonce-deviceinterface | pml-storeonce-deviceinterface |
| STO-4100 | Baseline Code Coverage — pml-storeonce-dashboard | pml-storeonce-dashboard |
| STO-4099 | Baseline Code Coverage — pml-storeonce-d2d-service | pml-storeonce-d2d-service |
| STO-4098 | Baseline Code Coverage — pml-storeonce-cat | pml-storeonce-cat |
| STO-4097 | Baseline Code Coverage — pml-storeonce-cache | pml-storeonce-cache |
| STO-4096 | Baseline Code Coverage — pml-storeonce-appliance | pml-storeonce-appliance |
| STO-4095 | Baseline Code Coverage — so-pml-pmle-mgmt | so-pml-pmle-mgmt |

### Catalyst Plugin Bugs (2020–2026)
| Ticket | Title | Repos Affected |
|--------|-------|----------------|
| STO-4540 | 5.2.x Image build failing due to missing rpms | pml-storeonce-fc, pml-storeonce-appliance |
| STO-4020 | StoreOnceCatalystCopy -force option non-interactive | so-d2d-catalyst_plugins |
| STO-2234 | Config bundles show incorrect values | pml-storeonce-cat |
| STO-1223 | Include all required files in Support ticket | so-D2D-Stack |
| STO-1193 | SQL plugin 1.6.0 platform architecture / discovery fix | so-d2d-catalyst_plugins |
| Bug 138887 | SQL Plugin: Concurrent SQL backups failure (Autoroutes) | so-d2d-catalyst_plugins |
| Bug 133697 | SAP-HANA Backint v1.50 support | so-d2d-catalyst_plugins |
| Bug 133714 | RMAN — Windows Oracle install config issue | so-d2d-catalyst_plugins |
| Bug 133233 | Object expiration not always successful | so-d2d-catalyst_plugins |
| Bug 133224 | Backup Exec OST client-side dedup support | so-d2d-catalyst_plugins |
| Bug 130966 | SQL plugin installation failure (Swissgrid) | so-d2d-catalyst_plugins |

### Parametrics & Resource Monitoring (2024–2025)
| Ticket | Title | Repos Affected |
|--------|-------|----------------|
| Bug 140069 / AT-39822 | Add Latency & Utilization to Disk Parametrics | so-D2D-Stack, d2d-rpc-fme, pml-storeonce-resourcemonitor |
| Bug 139972 / AT-39799 | Parametric Files in Comprehensive Support Bundles | CAT, NAS, REP, VTL, SMM, ResMon |
| STO-364 | Resolve inconsistency in parametric file locations | resourcemonitor, vtl, smm |

### Install/Update & System Config (2019–2024)
| Ticket | Title | Repos Affected |
|--------|-------|----------------|
| Bug 140166 | Install-update unit tests failing in Rocky (VDUOS 4.x) | so-pml-install-update |
| Bug 133848 | TPM Configuration during manufacturing | d2d4-bios-settings |
| Bug 133008 | systemd-journald watchdog timer configuration | d2d4-product-config, so-pml-install-update |
| Bug 132487 | 4.2.3 upgrade failure (GIA Informatik customer) | d2d4-product-config, so-pml-install-update |
| Bug 127190 | upgradefs -revert failure on passive PML vm node | so-pml-install-update |

### Log Collection (2019–2024)
| Ticket | Title | Repos Affected |
|--------|-------|----------------|
| Bug 136608 | Info missing from ticket Viewer for Delta Ticket | so-pml-logcollection |
| Bug 136561 | Corrected ticket collection start time | so-pml-logcollection |
| Bug 136553 | Delta log collection framework | so-pml-logcollection, pml-storeonce-d2d-service, so-D2D-Stack |
| Bug 138647 | Support Ticket auto-deletion fix | so-pml-logcollection |
| Bug 138617 | Log retention & size limit configuration | so-pml-logcollection |
| Bug 138600 | Modular update config/log files inclusion | so-pml-logcollection |
| AT-24709 | Include RMC pgsql logs in COMP support ticket | so-pml-logcollection |
| AT-35021 | Include data-gatherer logs in cVSA support ticket | so-pml-logcollection |
| Bug 128039 | Usage ticket stripping large files | so-pml-logcollection |

### MFA / 2FA (2023)
| Ticket | Title | Repos Affected |
|--------|-------|----------------|
| Bug 137434 | 2FA TOTP Localization | storeonce-grommet-ui |
| Bug 137481 | Reset 2FA button visibility | storeonce-grommet-ui |
| Bug 137416 | MFA enable/disable toggle behavior | storeonce-grommet-ui |
| Bug 137417 | 2FA permission-based access control | storeonce-grommet-ui |

### Other
| Ticket | Title | Repos Affected |
|--------|-------|----------------|
| Bug 133295 | SW update failure (Gedeon Richter customer) | so-pml-core |
| Bug 128869 | Updatemanager vetoing update | so-pml-updatemanager |
| Bug 124341 | (PML core fix) | so-pml-core |
| Bug 124036 | Update failing — NullPointerException in RSVS plugin | so-pml-hpsp-rsvs |
| Bug 113041 | Certificate expiry alert failure | so-pml-core |

---

## Total Unique Jira Tickets: **55+**

### Breakdown by Type
| Category | Count |
|----------|-------|
| Security (Dual Auth + MFA) | 19 |
| Code Quality (Coverage + Docs) | 9 |
| Catalyst Plugin (Bug/Feature) | 11 |
| Parametrics & Monitoring | 5 |
| Install/Update & System Config | 5 |
| Log Collection | 9 |
| Other (Core/Platform) | 5 |
