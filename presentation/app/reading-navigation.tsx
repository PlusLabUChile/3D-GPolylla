"use client";

import { useEffect, useState } from "react";

interface NavigationItem {
  id: string;
  index: number;
  label: string;
}

export function ReadingNavigation({ items }: { items: readonly NavigationItem[] }) {
  const [activeId, setActiveId] = useState(items[0]?.id ?? "");
  const [progress, setProgress] = useState(0);

  useEffect(() => {
    const updateProgress = () => {
      const scrollable = document.documentElement.scrollHeight - window.innerHeight;
      const next = scrollable <= 0 ? 100 : (window.scrollY / scrollable) * 100;
      setProgress(Math.min(100, Math.max(0, next)));
    };

    const observer = new IntersectionObserver(
      (entries) => {
        const visible = entries
          .filter((entry) => entry.isIntersecting)
          .sort((left, right) => right.intersectionRatio - left.intersectionRatio)[0];

        if (visible?.target.id) {
          setActiveId(visible.target.id);
        }
      },
      { rootMargin: "-20% 0px -60%", threshold: [0.1, 0.35, 0.65] },
    );

    for (const item of items) {
      const section = document.getElementById(item.id);
      if (section) {
        observer.observe(section);
      }
    }

    updateProgress();
    window.addEventListener("scroll", updateProgress, { passive: true });
    window.addEventListener("resize", updateProgress);

    return () => {
      observer.disconnect();
      window.removeEventListener("scroll", updateProgress);
      window.removeEventListener("resize", updateProgress);
    };
  }, [items]);

  const active = items.find((item) => item.id === activeId) ?? items[0];

  return (
    <>
      <div aria-label="Progreso de lectura" className="reading-progress" role="progressbar">
        <span style={{ width: `${progress}%` }} />
      </div>
      <aside aria-label="Navegación de lectura" className="toc">
        <p className="toc__title">Rediseño 3D-GPolylla</p>
        <ol>
          {items.map((item) => (
            <li key={item.id}>
              <a
                aria-current={activeId === item.id ? "location" : undefined}
                href={`#${item.id}`}
              >
                <span>{String(item.index).padStart(2, "0")}</span>
                {item.label}
              </a>
            </li>
          ))}
        </ol>
      </aside>
      <details className="mobile-nav">
        <summary>
          {String(active?.index ?? 1).padStart(2, "0")} · {active?.label}
        </summary>
        <nav aria-label="Capítulos">
          {items.map((item) => (
            <a href={`#${item.id}`} key={item.id}>
              {item.label}
            </a>
          ))}
        </nav>
      </details>
    </>
  );
}
