//! Wall-clock timings for a release, printed after execution and cleanup.

use std::fmt::Write;
use std::time::Duration;

use anyhow::Result;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub(super) enum Status {
    Ok,
    Failed,
    Skipped,
    Echoed,
}

impl Status {
    pub fn from_result<T>(result: &Result<T>) -> Self {
        if result.is_ok() {
            Self::Ok
        } else {
            Self::Failed
        }
    }

    fn label(self) -> &'static str {
        match self {
            Self::Ok => "ok",
            Self::Failed => "failed",
            Self::Skipped => "skipped",
            Self::Echoed => "echoed",
        }
    }
}

pub(super) struct Timing {
    pub label: String,
    pub status: Status,
    /// Skipped steps have no execution time.
    pub elapsed: Option<Duration>,
}

impl Timing {
    pub fn skipped(label: String) -> Self {
        Self {
            label,
            status: Status::Skipped,
            elapsed: None,
        }
    }
}

pub(super) struct JobReport {
    pub timing: Timing,
    pub steps: Vec<Timing>,
}

#[derive(Default)]
pub(super) struct Report {
    pub jobs: Vec<JobReport>,
}

impl Report {
    pub fn render(&self, elapsed: Duration, status: Status, dry_run: bool) -> String {
        let mode = if dry_run { " (dry run)" } else { "" };
        let mut output = format!("\nBuild report{mode}\n");
        writeln!(
            output,
            "Job rows are subtotals; details plus pipeline overhead add up to the total."
        )
        .unwrap();
        writeln!(output, "{:<8}  {:>15}  Job / step", "Status", "Time").unwrap();
        for job in &self.jobs {
            write_timing(&mut output, &job.timing, "");
            for step in &job.steps {
                write_timing(&mut output, step, "  ");
            }
            if let Some(elapsed) = job.timing.elapsed {
                let step_time: Duration = job.steps.iter().filter_map(|s| s.elapsed).sum();
                write_row(
                    &mut output,
                    "",
                    Some(elapsed.saturating_sub(step_time)),
                    "job overhead (setup, synchronization, bookkeeping)",
                    "  ",
                );
            }
        }
        let job_time: Duration = self.jobs.iter().filter_map(|j| j.timing.elapsed).sum();
        write_row(
            &mut output,
            "",
            Some(elapsed.saturating_sub(job_time)),
            "pipeline overhead",
            "",
        );
        writeln!(
            output,
            "\nTotal: {} ({})",
            format_duration(elapsed),
            status.label()
        )
        .unwrap();
        crate::system::process::redacted(&output)
    }
}

fn write_timing(output: &mut String, timing: &Timing, indent: &str) {
    write_row(
        output,
        timing.status.label(),
        timing.elapsed,
        &timing.label,
        indent,
    );
}

fn write_row(
    output: &mut String,
    status: &str,
    elapsed: Option<Duration>,
    label: &str,
    indent: &str,
) {
    let elapsed = elapsed
        .map(format_duration)
        .unwrap_or_else(|| "-".to_string());
    // Keep names and matrix values on one line, even when they contain newlines.
    let label = label.split_whitespace().collect::<Vec<_>>().join(" ");
    writeln!(output, "{:<8}  {:>15}  {indent}{label}", status, elapsed).unwrap();
}

fn format_duration(elapsed: Duration) -> String {
    // Round once before splitting the units, including carries at minute/hour
    // boundaries. Keep the same precision for step, job and overall timings.
    let millis = (elapsed.as_nanos() + 500_000) / 1_000_000;
    let seconds = millis / 1000;
    if seconds >= 3600 {
        format!(
            "{}h {:02}m {:02}.{:03}s",
            seconds / 3600,
            seconds / 60 % 60,
            seconds % 60,
            millis % 1000
        )
    } else if seconds >= 60 {
        format!(
            "{}m {:02}.{:03}s",
            seconds / 60,
            seconds % 60,
            millis % 1000
        )
    } else {
        format!("{}.{:03}s", seconds, millis % 1000)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn long_durations_preserve_precision_and_round_across_unit_boundaries() {
        for (micros, expected) in [
            (0, "0.000s"),
            (59_999_499, "59.999s"),
            (59_999_500, "1m 00.000s"),
            (372_123_000, "6m 12.123s"),
            (1_193_456_000, "19m 53.456s"),
            (3_599_999_500, "1h 00m 00.000s"),
            (3_661_234_000, "1h 01m 01.234s"),
        ] {
            assert_eq!(format_duration(Duration::from_micros(micros)), expected);
        }
    }

    #[test]
    fn accounts_for_job_and_pipeline_overhead_even_after_failure() {
        let report = Report {
            jobs: vec![
                JobReport {
                    timing: Timing {
                        label: "appimage".into(),
                        status: Status::Failed,
                        elapsed: Some(Duration::from_millis(54_145)),
                    },
                    steps: vec![
                        Timing {
                            label: "build AppImage".into(),
                            status: Status::Failed,
                            elapsed: Some(Duration::from_millis(47_171)),
                        },
                        Timing::skipped("conditional step".into()),
                    ],
                },
                JobReport {
                    timing: Timing::skipped("conditional job".into()),
                    steps: Vec::new(),
                },
            ],
        };
        let rendered = report.render(Duration::from_millis(55_500), Status::Failed, false);
        let overhead = |label| {
            rendered
                .lines()
                .find(|line| line.starts_with(' ') && line.contains(label))
                .unwrap()
        };
        assert!(overhead("job overhead").contains("6.974s"), "{rendered}");
        assert!(
            overhead("pipeline overhead").contains("1.355s"),
            "{rendered}"
        );
        assert_eq!(rendered.matches("job overhead").count(), 1);
        assert!(rendered.ends_with("Total: 55.500s (failed)\n"));
    }

    #[test]
    fn accounts_for_time_before_any_job_starts() {
        let rendered = Report::default().render(Duration::from_secs(2), Status::Failed, false);
        assert!(
            rendered
                .lines()
                .any(|line| { line.ends_with("pipeline overhead") && line.contains("2.000s") }),
            "{rendered}"
        );
        assert!(rendered.ends_with("Total: 2.000s (failed)\n"));
    }
}
