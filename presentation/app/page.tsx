import { sections } from "./presentation-data";
import { ReadingNavigation } from "./reading-navigation";
import { SectionVisual } from "./section-visual";

const sourceDocument =
  "https://github.com/PlusLabUChile/3D-GPolylla/blob/main/docs/superpowers/specs/2026-07-28-gpolylla-redesign-design.md";

export default function Home() {
  const navigationItems = sections.map(({ id, index, label }) => ({ id, index, label }));

  return (
    <>
      <a className="skip-link" href="#content">
        Saltar al contenido
      </a>
      <ReadingNavigation items={navigationItems} />
      <main className="presentation" id="content">
        {sections.map((section) => (
          <section
            aria-labelledby={`${section.id}-title`}
            className={`chapter${section.index === 1 ? " chapter--hero" : ""}`}
            data-presentation-section
            id={section.id}
            key={section.id}
          >
            <header>
              <p className="chapter__number">
                Capítulo {String(section.index).padStart(2, "0")} · {section.label}
              </p>
              {section.index === 1 ? (
                <h1 id={`${section.id}-title`}>{section.title}</h1>
              ) : (
                <h2 id={`${section.id}-title`}>{section.title}</h2>
              )}
              <p className="chapter__thesis">{section.thesis}</p>
            </header>
            <SectionVisual kind={section.visual} />
            <ul className="chapter__points">
              {section.bullets.map((bullet) => (
                <li key={bullet}>{bullet}</li>
              ))}
            </ul>
            {section.detail ? (
              <details>
                <summary>Profundizar</summary>
                <p>{section.detail}</p>
              </details>
            ) : null}
          </section>
        ))}
        <footer className="presentation-footer">
          <h2>Documento técnico</h2>
          <p>
            La presentación resume las decisiones. El documento versionado conserva el
            razonamiento, los riesgos y los criterios completos.
          </p>
          <a
            className="source-link"
            href={sourceDocument}
            rel="noreferrer"
            target="_blank"
          >
            Leer el documento completo
          </a>
        </footer>
      </main>
    </>
  );
}
