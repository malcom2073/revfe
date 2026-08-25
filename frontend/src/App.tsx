import { BrowserRouter, Route, Routes } from "react-router-dom";
import AppLayout from "./app/AppLayout";
import Dashboard from "./pages/Dashboard";
import Instances from "./pages/Instances";
import InstanceDetail from "./pages/InstanceDetail";
import Images from "./pages/Images";
import Storage from "./pages/Storage";
import Networks from "./pages/Networks";
import Profiles from "./pages/Profiles";

export default function App() {
  return (
    <BrowserRouter>
      <AppLayout>
        <Routes>
          <Route path="/" element={<Dashboard />} />
          <Route path="/instances" element={<Instances />} />
          <Route path="/instances/:name" element={<InstanceDetail />} />
          <Route path="/images" element={<Images />} />
          <Route path="/storage" element={<Storage />} />
          <Route path="/networks" element={<Networks />} />
          <Route path="/profiles" element={<Profiles />} />
        </Routes>
      </AppLayout>
    </BrowserRouter>
  );
}
