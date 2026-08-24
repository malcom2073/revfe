import { useEffect, useState } from "react";
import { NavLink, useLocation } from "react-router-dom";
import {
  Masthead,
  MastheadBrand,
  MastheadContent,
  MastheadMain,
  Nav,
  NavItem,
  NavList,
  Page,
  PageSidebar,
  PageSidebarBody,
  Spinner,
  Toolbar,
  ToolbarContent,
  ToolbarItem,
} from "@patternfly/react-core";
import {
  ServerIcon,
  CubesIcon,
  DatabaseIcon,
  NetworkWiredIcon,
  LayerGroupIcon,
  CogIcon,
} from "@patternfly/react-icons";
import { api } from "../api/client";
import type { ServerInfo } from "../api/types";

const navItems = [
  { label: "Dashboard", to: "/", icon: <ServerIcon /> },
  { label: "Instances", to: "/instances", icon: <CubesIcon /> },
  { label: "Images", to: "/images", icon: <LayerGroupIcon />, disabled: false },
  { label: "Storage", to: "/storage", icon: <DatabaseIcon />, disabled: false },
  { label: "Networks", to: "/networks", icon: <NetworkWiredIcon />, disabled: true },
  { label: "Settings", to: "/settings", icon: <CogIcon />, disabled: true },
];

export default function AppLayout({
  children,
}: {
  children: React.ReactNode;
}) {
  const [info, setInfo] = useState<ServerInfo | null>(null);
  const [error, setError] = useState<string | null>(null);
  const location = useLocation();

  useEffect(() => {
    api
      .serverInfo()
      .then(setInfo)
      .catch((e) => setError(e.message));
  }, []);

  return (
    <Page
      masthead={
        <Masthead>
          <MastheadMain>
            <MastheadBrand>
              <span style={{ fontSize: "1.25rem", fontWeight: 600 }}>
                <ServerIcon /> RevFe
              </span>
            </MastheadBrand>
          </MastheadMain>
          <MastheadContent>
            <Toolbar>
              <ToolbarContent>
                <ToolbarItem>
                  {info ? (
                    info.auth === "untrusted" ? (
                      <span style={{ color: "#f4a950" }}>
                        Certificate not trusted — run <code>make setup-cert</code>
                      </span>
                    ) : (
                      <span>
                        <strong>{info.name}</strong> · Incus {info.version} ·
                        project <em>{info.project}</em>
                      </span>
                    )
                  ) : error ? (
                    <span style={{ color: "#f4a950" }}>{error}</span>
                  ) : (
                    <Spinner size="sm" aria-label="Loading server info" />
                  )}
                </ToolbarItem>
              </ToolbarContent>
            </Toolbar>
          </MastheadContent>
        </Masthead>
      }
      sidebar={
        <PageSidebar>
          <PageSidebarBody>
            <Nav>
              <NavList>
                {navItems.map((item) =>
                  item.disabled ? (
                    <NavItem
                      key={item.to}
                      isActive={false}
                      disabled
                      to={item.to}
                    >
                      {item.icon} {item.label}
                    </NavItem>
                  ) : (
                    <NavItem key={item.to} isActive={location.pathname === item.to}>
                      <NavLink to={item.to} end={item.to === "/"}>
                        {item.icon} {item.label}
                      </NavLink>
                    </NavItem>
                  )
                )}
              </NavList>
            </Nav>
          </PageSidebarBody>
        </PageSidebar>
      }
    >
      {children}
    </Page>
  );
}
