import type { Metadata } from "next";
import "./globals.css";

export const metadata: Metadata = {
  title: "Rediseño de 3D-GPolylla",
  description:
    "Presentación autónoma de la arquitectura propuesta para consolidar una familia de algoritmos de generación de poliedros.",
};

export default function RootLayout({ children }: Readonly<{ children: React.ReactNode }>) {
  return (
    <html lang="es">
      <body>{children}</body>
    </html>
  );
}
